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

/* ═══════════════════════════════════════════════════════════
   LCG PRNG  (fast, deterministic, seed-driven)
   Knuth multiplicative LCG: x_{n+1} = x_n * 6364136223846793005 + 1442695040888963407
═══════════════════════════════════════════════════════════ */

uint64_t gen_lcg_next(uint64_t *state) {
    *state = *state * UINT64_C(6364136223846793005) + UINT64_C(1442695040888963407);
    return *state;
}

int gen_rand_range(uint64_t *rng, int min_incl, int max_incl) {
    if (min_incl >= max_incl) return min_incl;
    uint64_t r = gen_lcg_next(rng);
    return min_incl + (int)(r % (uint64_t)(max_incl - min_incl + 1));
}

/* ═══════════════════════════════════════════════════════════
   SEED GENERATION
   160-bit value displayed as 40 hex chars (ETH address width).
   Internally we store 3 x uint64 (192 bits) and truncate display.
═══════════════════════════════════════════════════════════ */

/* FNV-1a 64 on arbitrary bytes */
static uint64_t fnv1a_64(const void *data, size_t len) {
    const uint8_t *p = (const uint8_t *)data;
    uint64_t hash = UINT64_C(14695981039346656037);
    for (size_t i = 0; i < len; i++) {
        hash ^= p[i];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static void seed_fill_hex(Seed *s) {
    /* Use first 160 bits (parts[0] + parts[1] + lower 32 bits of parts[2])
       Displayed as 40 hex chars like an Ethereum address */
    uint8_t raw[20];
    /* big-endian packing */
    for (int i = 0; i < 8; i++) raw[i]     = (uint8_t)(s->parts[0] >> (56 - 8*i));
    for (int i = 0; i < 8; i++) raw[8+i]   = (uint8_t)(s->parts[1] >> (56 - 8*i));
    for (int i = 0; i < 4; i++) raw[16+i]  = (uint8_t)(s->parts[2] >> (24 - 8*i));
    for (int i = 0; i < 20; i++) {
        snprintf(s->hex + 2*i, 3, "%02x", raw[i]);
    }
    s->hex[SEED_HEX_LEN] = '\0';
}

Seed gen_random_seed(void) {
    Seed s;
    uint64_t t = (uint64_t)time(NULL) ^ (uint64_t)clock();
    uint64_t e = fnv1a_64(&t, sizeof(t));
    s.parts[0] = fnv1a_64(&e, sizeof(e));
    s.parts[1] = fnv1a_64(&s.parts[0], sizeof(s.parts[0]));
    s.parts[2] = fnv1a_64(&s.parts[1], sizeof(s.parts[1]));
    seed_fill_hex(&s);
    return s;
}

Seed gen_seed_from_u64(uint64_t raw) {
    Seed s;
    s.parts[0] = fnv1a_64(&raw, sizeof raw);
    s.parts[1] = fnv1a_64(&s.parts[0], sizeof s.parts[0]);
    s.parts[2] = fnv1a_64(&s.parts[1], sizeof s.parts[1]);
    seed_fill_hex(&s);
    return s;
}

Seed gen_seed_from_hex(const char *hex) {
    /* parse up to 40 hex chars back into parts */
    Seed s;
    memset(&s, 0, sizeof s);
    size_t len = strlen(hex);
    if (len > SEED_HEX_LEN) len = SEED_HEX_LEN;
    uint8_t raw[20] = {0};
    for (size_t i = 0; i + 1 < len; i += 2) {
        unsigned v;
        sscanf(hex + i, "%02x", &v);
        raw[i/2] = (uint8_t)v;
    }
    for (int i = 0; i < 8; i++) s.parts[0] |= ((uint64_t)raw[i])    << (56 - 8*i);
    for (int i = 0; i < 8; i++) s.parts[1] |= ((uint64_t)raw[8+i])  << (56 - 8*i);
    for (int i = 0; i < 4; i++) s.parts[2] |= ((uint64_t)raw[16+i]) << (24 - 8*i);
    seed_fill_hex(&s);
    return s;
}

/* Derive a single uint64 working RNG state from the seed */
static uint64_t seed_to_rng(const Seed *s) {
    uint64_t r = s->parts[0] ^ (s->parts[1] * UINT64_C(0xdeadbeefcafe1337)) ^ s->parts[2];
    /* warm up */
    gen_lcg_next(&r);
    gen_lcg_next(&r);
    return r;
}

/* ═══════════════════════════════════════════════════════════
   NOISE GENERATION  (Diamond-Square / midpoint displacement
   simplified to a two-pass octave noise for zone formation)
═══════════════════════════════════════════════════════════ */

/* Generate a float heightmap [0,1] using summed octave hash noise */
static float *gen_heightmap(int W, int H, uint64_t *rng) {
    float *hmap = malloc(W * H * sizeof(float));
    if (!hmap) { perror("malloc"); exit(1); }

    /* Seed a simple noise table */
    float noise_table[256];
    for (int i = 0; i < 256; i++)
        noise_table[i] = (float)(gen_lcg_next(rng) & 0xFFFF) / 65535.0f;

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
        }
    }
    return hmap;
}

/* ═══════════════════════════════════════════════════════════
   TILE ALLOCATION
═══════════════════════════════════════════════════════════ */

static Tile **alloc_tiles(int W, int H) {
    Tile **tiles = malloc(H * sizeof(Tile *));
    if (!tiles) { perror("malloc"); exit(1); }
    for (int y = 0; y < H; y++) {
        tiles[y] = malloc(W * sizeof(Tile));
        if (!tiles[y]) { perror("malloc"); exit(1); }
        for (int x = 0; x < W; x++) {
            tiles[y][x].terrain = TERRAIN_PLAIN;
            tiles[y][x].city_id = -1;
            tiles[y][x].camp_id = -1;
        }
    }
    return tiles;
}

/* ═══════════════════════════════════════════════════════════
   CHEBYSHEV DISTANCE
═══════════════════════════════════════════════════════════ */

static int chebyshev(int ax, int ay, int bx, int by) {
    int dx = ax - bx; if (dx < 0) dx = -dx;
    int dy = ay - by; if (dy < 0) dy = -dy;
    return dx > dy ? dx : dy;
}

/* ═══════════════════════════════════════════════════════════
   MAP GENERATION MAIN
═══════════════════════════════════════════════════════════ */

void gen_map(GameState *gs) {
    int W = gs->map.width;
    int H = gs->map.height;
    uint64_t rng = seed_to_rng(&gs->seed);

    gs->map.tiles = alloc_tiles(W, H);

    /* ── 1. Height + moisture noise ──────────────────────── */
    float *hmap  = gen_heightmap(W, H, &rng);
    float *mmap  = gen_heightmap(W, H, &rng); /* moisture */

    /* ── 2. Classify terrain from height + moisture ──────── */
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

    /* ── 3. Pick starting plain tile (center-ish) ─────────── */
    int sx = W / 2, sy = H / 2;
    /* Search outward in spiral until we find a PLAIN tile */
    for (int r = 0; r <= (W > H ? W : H); r++) {
        int found = 0;
        for (int dy = -r; dy <= r && !found; dy++) {
            for (int dx = -r; dx <= r && !found; dx++) {
                if ((dx != -r && dx != r) && (dy != -r && dy != r)) continue; /* only ring */
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
        int cx = gen_rand_range(&rng, 0, W - 1);
        int cy = gen_rand_range(&rng, 0, H - 1);
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