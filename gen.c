#include "gen.h"
#include "config.h"
#include "structs.h"
#include "units.h"
#include "cities.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

void compute_hex(Seed *s) {
    static const char *h = "0123456789abcdef";
    uint64_t v = s->value;
    for (int i = SEED_HEX_LEN - 1; i >= 0; --i) {
        s->hex[i] = h[v & 0xF];
        v >>= 4;
    }
    s->hex[SEED_HEX_LEN] = '\0';
}

Seed gen_seed(char *str) {
    Seed s;
    uint64_t hash = 14695981039346656037ULL;
    if (str) {
        while (*str) {
            hash ^= (uint8_t)*str++;
            hash *= 1099511628211ULL;
        }
    }
    s.value = hash;
    compute_hex(&s);
    return s;
}

Seed gen_random_seed(void) {
    Seed s;
    uint64_t x = (uint64_t)time(NULL);
    x ^= (uint64_t)(uintptr_t)&s;
    x ^= (uint64_t)clock();
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    s.value = x;
    compute_hex(&s);
    return s;
}

uint64_t mix(uint64_t x) {
    return x * UINT64_C(6364136223846793005) + UINT64_C(1442695040888963407);
}

int gen_rand_range(uint64_t rng, int min_incl, int max_incl) {
    if (min_incl >= max_incl) return min_incl;
    uint64_t r = mix(rng);
    return min_incl + (int)(r % (uint64_t)(max_incl - min_incl + 1));
}

float *gen_heightmap(int W, int H, uint64_t rng) {
    float *hmap = malloc(W * H * sizeof(float));
    if (!hmap) { perror("malloc"); exit(1); }
    float noise_table[256];
    for (int i = 0; i < 256; i++) {
        rng = mix(rng);
        noise_table[i] = (float)(mix(rng) & 0xFFFF) / 65535.0f;
    }
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            float v = 0.0f;
            float amp = 1.0f, freq = 1.0f, total_amp = 0.0f;
            for (int o = 0; o < 4; o++) {
                int ix = (int)(x * freq) & 255;
                int iy = (int)(y * freq) & 255;
                int idx = (ix * 31 + iy * 17 + o * 97) & 255;
                v += noise_table[idx] * amp;
                total_amp += amp;
                amp  *= 0.5f;
                freq *= 2.0f;
            }
            hmap[y * W + x] = v / total_amp;
        }}
    return hmap;
}

Tile **alloc_tiles(int W, int H) {
    Tile **tiles = malloc(H * sizeof(Tile *));
    if (!tiles) { perror("malloc"); exit(1); }
    for (int y = 0; y < H; y++) {
        tiles[y] = malloc(W * sizeof(Tile));
        if (!tiles[y]) { perror("malloc"); exit(1); }
        for (int x = 0; x < W; x++) {
            tiles[y][x].terrain = TERRAIN_PLAIN;
            tiles[y][x].city_id = -1;
            tiles[y][x].camp_id = -1;
        }}
    return tiles;
}

void gen_map(GameState *gs) {
    int W = gs->map.width;
    int H = gs->map.height;
    gs->map.tiles = alloc_tiles(W, H);
    uint64_t rng = mix(mix(gs->seed.value));
    float *hmap  = gen_heightmap(W, H, rng);
    float *mmap  = gen_heightmap(W, H, mix(rng));
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            float h = hmap[y * W + x];
            float m = mmap[y * W + x];
            TerrainType t;
            if (h < 0.28f) {
                t = TERRAIN_WATER;
            } else if (h < 0.40f) {
                t = (m > 0.55f) ? TERRAIN_TUNDRA : TERRAIN_DESERT;
            } else if (h < 0.60f) {
                t = (m > 0.50f) ? TERRAIN_FOREST : TERRAIN_PLAIN;
            } else if (h < 0.80f) {
                t = (m > 0.45f) ? TERRAIN_FOREST : TERRAIN_PLAIN;
            } else {
                t = TERRAIN_MOUNTAIN;
            }
            gs->map.tiles[y][x].terrain = t;
        }
    }
    free(hmap);
    free(mmap);

    int sx = W / 2, sy = H / 2;
    for (int r = 0; r <= (W > H ? W : H); r++) {
        int found = 0;
        for (int dy = -r; dy <= r && !found; dy++) {
            for (int dx = -r; dx <= r && !found; dx++) {
                if ((dx != -r && dx != r) && (dy != -r && dy != r)) continue;
                int nx = sx + dx, ny = sy + dy;
                if (nx < 0 || nx >= W || ny < 0 || ny >= H) continue;
                if (gs->map.tiles[ny][nx].terrain == TERRAIN_PLAIN) {
                    sx = nx; sy = ny; found = 1;
                }
            }
        }
        if (found) break;
        /* if r=0 and center is plain, already done */
        if (r == 0 && gs->map.tiles[sy][sx].terrain == TERRAIN_PLAIN) break;
    }
    /* Force to plain if nothing found */
    gs->map.tiles[sy][sx].terrain = TERRAIN_PLAIN;

    /* ── 4. Place starting city ──────────────────────────── */
    gs->next_city_id = 0;
    City *start_city = city_create(gs, sx, sy, 0 /* founded_turn */);
    (void)start_city;

    /* ── 5. Place starting warrior on the city tile ──────── */
    gs->next_unit_id = 0;
    unit_create(gs, UNIT_TYPE_WARRIOR, UNIT_OWNER_PLAYER, sx, sy);

    /* ── 6. Place barbarian camps ─────────────────────────── */
    gs->next_camp_id = 0;
    int placed = 0;
    int attempts = 0;
    while (placed < gs->n_barbarian_camps_initial && attempts < W * H * 4) {
        attempts++;
        int cx = gen_rand_range(rng, 0, W - 1);
        int cy = gen_rand_range(rng, 0, H - 1);
        /* must be land, not water */
        if (gs->map.tiles[cy][cx].terrain == TERRAIN_WATER) continue;
        /* must be > BARB_MIN_DIST_FROM_START from start */
        if (chebyshev(cx, cy, sx, sy) < BARB_MIN_DIST_FROM_START) continue;
        /* must not already have a camp or city */
        if (gs->map.tiles[cy][cx].camp_id != -1) continue;
        if (gs->map.tiles[cy][cx].city_id != -1) continue;

        /* allocate camp */
        Camp *c = malloc(sizeof(Camp));
        if (!c) { perror("malloc"); exit(1); }
        c->id    = gs->next_camp_id++;
        c->x     = cx;
        c->y     = cy;
        c->alive = 1;
        c->next  = gs->camps;
        gs->camps = c;
        gs->map.tiles[cy][cx].camp_id = c->id;
        placed++;
    }
}

Map gen_map(int W, int H, Seed seed);
void gen_first_city(GameState *gs);
void gen_camps(GameState *gs, int n);
GameState *gen_game(int width, int height, int turns, int n_barb_camps, Seed seed);
