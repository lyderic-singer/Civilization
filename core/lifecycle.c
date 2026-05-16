#include "core.h"

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

Map gen_map(int W, int H, Seed seed) {
    Map map;
    map.width = W;
    map.height = H;
    map.tiles = alloc_tiles(W, H);
    uint64_t rng = mix(mix(seed.value));
    float *hmap = gen_heightmap(W, H, rng);
    float *mmap = gen_heightmap(W, H, mix(rng));
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            float h = hmap[y * W + x];
            float m = mmap[y * W + x];
            TerrainType t;
            if (h < 0.28f) {
    t = TERRAIN_WATER;
} else if (h < 0.40f) {
    t = (m > 0.60f) ? TERRAIN_TUNDRA : TERRAIN_DESERT;
} else if (h < 0.60f) {
    t = (m > 0.60f) ? TERRAIN_FOREST : TERRAIN_PLAIN;
} else if (h < 0.80f) {
    t = (m > 0.55f) ? TERRAIN_FOREST : TERRAIN_PLAIN;
} else {
    t = TERRAIN_MOUNTAIN;
}
            map.tiles[y][x].terrain = t;
    }}
    free(hmap);
    free(mmap);
    return map;
}

void gen_first_city(GameState *gs) {
    int W = gs->map.width;
    int H = gs->map.height;
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
        }}}
        if (found) break;
        if (r == 0 && gs->map.tiles[sy][sx].terrain == TERRAIN_PLAIN) break;
    }
    gs->map.tiles[sy][sx].terrain = TERRAIN_PLAIN;
    gs->next_city_id = 0;
    city_create(gs, sx, sy, 0);
    gs->next_unit_id = 0;
    unit_create(gs, UNIT_TYPE_WARRIOR, UNIT_OWNER_PLAYER, sx, sy);
}

int camps_count(GameState *gs) {
    int n = 0;
    for (Camp *c = gs->camps; c; c = c->next) n++;
    return n;
}

void gen_camps(GameState *gs, int n) {
    int W = gs->map.width;
    int H = gs->map.height;
    uint64_t rng = mix(mix(gs->seed.value));
    int sx = gs->cities->x;
    int sy = gs->cities->y;
    gs->next_camp_id = 0;
    int placed = 0, attempts = 0;
    while (placed < n && attempts < W * H * 4) {
        attempts++;
        rng = mix(rng);
        int cx = gen_rand_range(rng, 0, W - 1);
        rng = mix(rng);
        int cy = gen_rand_range(rng, 0, H - 1);
        if (gs->map.tiles[cy][cx].terrain == TERRAIN_WATER) continue;
        if (chebyshev(cx, cy, sx, sy) < BARB_MIN_DIST_FROM_START) continue;
        if (gs->map.tiles[cy][cx].camp_id != -1) continue;
        if (gs->map.tiles[cy][cx].city_id != -1) continue;
        Camp *c = malloc(sizeof(Camp));
        if (!c) { perror("malloc"); exit(1); }
        c->id = gs->next_camp_id++;
        c->x = cx;
        c->y = cy;
        c->alive = 1;
        c->next = gs->camps;
        gs->camps = c;
        gs->map.tiles[cy][cx].camp_id = c->id;
        placed++;
    }
}

GameState *gen_game(int width, int height, int turns, int n_barb_camps, Seed seed) {
    GameState *gs = calloc(1, sizeof(GameState));
    if (!gs) { perror("calloc"); exit(1); }
    gs->map.width = width;
    gs->map.height = height;
    gs->turn = 1;
    gs->turn_max = turns;
    gs->n_barbarian_camps_initial = n_barb_camps;
    gs->n_camps_destroyed = 0;
    gs->barb_unit_count = 0;
    gs->gold = 10;
    gs->territorial_consecutive_turns = 0;
    gs->zero_prod_turns = 0;
    gs->result = RESULT_ONGOING;
    gs->next_unit_id = 0;
    gs->next_city_id = 0;
    gs->next_camp_id = 0;
    gs->seed = seed;
    tech_init(&gs->tech);
    tech_apply_effects(gs);
    gs->map = gen_map(width, height, seed);
    gen_first_city(gs);
    gen_camps(gs, n_barb_camps);
    printf("\n=== Early Civilization ===\n");
    printf("Seed: 0x%s\n", gs->seed.hex);
    printf("Carte: %dx%d | Tours max: %d | Camps barbares: %d\n\n", width, height, turns, n_barb_camps);
    return gs;
}

void free_units(Unit *u) {
    while (u) { Unit *n = u->next; free(u); u = n; }
}

void free_cities(City *c) {
    while (c) { City *n = c->next; free(c); c = n; }
}

void free_camps(Camp *c) {
    while (c) { Camp *n = c->next; free(c); c = n; }
}

void free_map(Map *m) {
    if (!m->tiles) return;
    for (int y = 0; y < m->height; y++) free(m->tiles[y]);
    free(m->tiles);
    m->tiles = NULL;
}

void free_game(GameState *gs) {
    free_units(gs->units);
    free_cities(gs->cities);
    free_camps(gs->camps);
    free_map(&gs->map);
    free(gs);
}
