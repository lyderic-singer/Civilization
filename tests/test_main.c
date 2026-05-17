#include "../core/core.h"
#include "../core/turn.h"
#include <stdio.h>

static int passes = 0;
static int failures = 0;

#define ASSERT(cond) \
    if (cond) passes++; \
    else { failures++; printf("  FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); }

static void test_chebyshev(void) {
    ASSERT(chebyshev(0, 0, 0, 0) == 0);
    ASSERT(chebyshev(0, 0, 3, 0) == 3);
    ASSERT(chebyshev(0, 0, 0, 3) == 3);
    ASSERT(chebyshev(0, 0, 3, 3) == 3);
    ASSERT(chebyshev(0, 0, 2, 5) == 5);
    ASSERT(chebyshev(0, 0, 5, 2) == 5);
    ASSERT(chebyshev(5, 5, 2, 2) == 3);
    ASSERT(chebyshev(2, 3, 7, 8) == chebyshev(7, 8, 2, 3));
}

static void test_seed_determinism(void) {
    Seed s = gen_seed("1545998465132");
    int w = 20;
    int h = 15;
    GameState *gs1 = gen_game(w, h, 3, 3, s);
    GameState *gs2 = gen_game(w, h, 100, 3, s);

    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
            ASSERT(gs1->map.tiles[y][x].terrain == gs2->map.tiles[y][x].terrain);

    free_game(gs1);
    free_game(gs2);
}

static void test_colonise(void) {
    Seed s = gen_seed("46513216549845");
    GameState *gs = gen_game(30, 30, 100, 0, s);

    int cx = gs->cities->x;
    int cy = gs->cities->y;
    int tx = (cx + 10 < 30) ? cx + 10 : cx - 10;
    int ty = (cy + 10 < 30) ? cy + 10 : cy - 10;

    gs->map.tiles[ty][tx].terrain = TERRAIN_PLAIN;
    gs->map.tiles[ty][tx].city_id = -1;
    gs->map.tiles[ty][tx].camp_id = -1;

    int city_count_before = cities_count(gs);
    Unit *settler = unit_create(gs, UNIT_TYPE_SETTLER, UNIT_OWNER_PLAYER, tx, ty);

    ASSERT(game_settler_colonise(gs, settler) == 1);
    ASSERT(cities_count(gs) == city_count_before + 1);
    ASSERT(unit_by_id(gs, settler->id) == NULL);

    Unit *settler2 = unit_create(gs, UNIT_TYPE_SETTLER, UNIT_OWNER_PLAYER, tx, ty);
    ASSERT(game_settler_colonise(gs, settler2) == 0);

    int wx = 0;
    int wy = 0;
    gs->map.tiles[wy][wx].terrain = TERRAIN_WATER;
    gs->map.tiles[wy][wx].city_id = -1;
    gs->map.tiles[wy][wx].camp_id = -1;
    Unit *settler3 = unit_create(gs, UNIT_TYPE_SETTLER, UNIT_OWNER_PLAYER, wx, wy);
    ASSERT(game_settler_colonise(gs, settler3) == 0);

    free_game(gs);
}

static void test_city_growth(void) {
    Seed s = gen_seed("4651312165412384796");
    GameState *gs = gen_game(20, 20, 100, 0, s);
    City *c = gs->cities;

    c->population = 1;
    c->yield_food = 2;

    int pop_before = c->population;
    c->food_stock = city_growth_threshold(c) - 1;
    cities_grow(gs);
    ASSERT(c->population == pop_before);

    c->food_stock = city_growth_threshold(c);
    cities_grow(gs);
    ASSERT(c->population == pop_before + 1);
    ASSERT(c->food_stock == 0);

    free_game(gs);
}

int main(void) {
    printf("Tests :\n");

    printf("chebyshev... "); test_chebyshev(); printf("ok\n");
    printf("seed determinism... "); test_seed_determinism(); printf("ok\n");
    printf("colonise... "); test_colonise(); printf("ok\n");
    printf("city growth... "); test_city_growth(); printf("ok\n");

    printf("\n%d passed, %d failed\n", passes, failures);
    return failures > 0 ? 1 : 0;
}
