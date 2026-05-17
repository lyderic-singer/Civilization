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

static void test_city_famine(void) {
    Seed s = gen_seed("49815327984615");
    GameState *gs = gen_game(20, 20, 100, 0, s);
    City *c = gs->cities;

    c->population = 2;
    c->yield_food = 0;
    c->food_stock = 0;
    cities_grow(gs);
    ASSERT(c->population == 1);

    free_game(gs);
}

static void test_production(void) {
    Seed s = gen_seed("4986557552134986");
    GameState *gs = gen_game(20, 20, 100, 0, s);
    City *c = gs->cities;

    c->yield_prod = 10;
    city_set_project(c, PROJECT_BUILDING, BUILDING_GRANARY);
    int cost = city_project_cost(gs, c);

    cities_produce(gs);
    ASSERT(c->prod_stock == 10);

    c->prod_stock = cost - 1;
    cities_produce(gs);
    ASSERT(c->buildings & BUILDING_GRANARY);
    ASSERT(c->prod_stock == 0);
    ASSERT(c->project_type == PROJECT_NONE);

    free_game(gs);
}

static void test_tech(void) {
    Seed s = gen_seed("79845611960");
    GameState *gs = gen_game(20, 20, 100, 0, s);

    ASSERT(tech_available(&gs->tech, TECH_ID_HUNTING) == 1);
    ASSERT(tech_available(&gs->tech, TECH_ID_FARMING) == 1);

    ASSERT(tech_available(&gs->tech, TECH_ID_IRRIGATION) == 0);

    gs->tech.acquired[TECH_ID_FARMING] = 1;
    ASSERT(tech_available(&gs->tech, TECH_ID_IRRIGATION) == 1);

    gs->tech.acquired[TECH_ID_HUNTING] = 1;
    ASSERT(tech_available(&gs->tech, TECH_ID_HUNTING) == 0);

    free_game(gs);
}

static void test_combat(void) {
    Seed s = gen_seed("497561231953416518");
    GameState *gs = gen_game(20, 20, 100, 0, s);

    Unit *attacker = unit_create(gs, UNIT_TYPE_WARRIOR, UNIT_OWNER_PLAYER, 5, 5);
    Unit *defender = unit_create(gs, UNIT_TYPE_WARRIOR, UNIT_OWNER_BARB, 6, 5);

    int atk_id = attacker->id;
    int def_id = defender->id;

    attacker->hp = attacker->hp_max;
    defender->hp = 1;

    int def_defense = defender->defense;
    unit_attack_unit(gs, attacker, defender);

    ASSERT(unit_by_id(gs, def_id) == NULL);
    ASSERT(unit_by_id(gs, atk_id) != NULL);

    int dmg = attacker->attack - def_defense;
    if (dmg < 1) dmg = 1;
    Unit *a2 = unit_create(gs, UNIT_TYPE_WARRIOR, UNIT_OWNER_PLAYER, 7, 5);
    Unit *d2 = unit_create(gs, UNIT_TYPE_WARRIOR, UNIT_OWNER_BARB, 8, 5);
    a2->hp = a2->hp_max;
    d2->hp = d2->hp_max;
    unit_attack_unit(gs, a2, d2);
    ASSERT(d2->hp == d2->hp_max - dmg);

    free_game(gs);
}

static void test_unit_move(void) {
    Seed s = gen_seed("143841235618");
    GameState *gs = gen_game(20, 20, 100, 0, s);

    Unit *u = unit_create(gs, UNIT_TYPE_WARRIOR, UNIT_OWNER_PLAYER, 5, 5);
    gs->map.tiles[5][6].terrain = TERRAIN_PLAIN;
    gs->map.tiles[5][7].terrain = TERRAIN_WATER;

    int pm_before = u->pm_left;
    ASSERT(unit_move(gs, u, 6, 5) == 1);
    ASSERT(u->x == 6 && u->y == 5);
    ASSERT(u->pm_left == pm_before - MOVE_COSTS[TERRAIN_PLAIN]);

    ASSERT(unit_move(gs, u, 7, 5) == 0);
    ASSERT(u->x == 6 && u->y == 5);

    u->pm_left = 0;
    ASSERT(unit_move(gs, u, 6, 4) == 0);

    free_game(gs);
}

static void test_game_result(void) {
    Seed s = gen_seed("41987134195312");
    GameState *gs = gen_game(20, 20, 10, 0, s);

    ASSERT(game_check_result(gs) == RESULT_ONGOING);

    City *c = gs->cities;
    city_raze(gs, c->id);
    ASSERT(game_check_result(gs) == RESULT_LOSS_NOCITY);

    free_game(gs);
    gs = gen_game(20, 20, 1, 0, s);
    gs->turn = 2;
    ASSERT(game_check_result(gs) == RESULT_LOSS_TURNS);

    free_game(gs);
    gs = gen_game(20, 20, 100, 0, s);
    for (int i = 0; i < TECH_COUNT; i++)
        gs->tech.acquired[i] = 1;
    ASSERT(game_check_result(gs) == RESULT_WIN_SCI);

    free_game(gs);
}

static void test_combat_city(void) {
    Seed s = gen_seed("7846153014698");
    GameState *gs = gen_game(20, 20, 100, 0, s);

    City *c = gs->cities;
    int city_id = c->id;

    Unit *attacker = unit_create(gs, UNIT_TYPE_WARRIOR, UNIT_OWNER_BARB, 5, 5);
    attacker->attack = 999;
    c->hp = 1;

    unit_attack_city(gs, attacker, c);

    ASSERT(city_by_id(gs, city_id) == NULL);

    free_game(gs);
}

static void test_barbarians(void) {
    Seed s = gen_seed("6451164578964312");
    GameState *gs = gen_game(20, 20, 100, 1, s);

    int barbs_before = gs->barb_unit_count;
    Camp *camp = gs->camps;
    gs->map.tiles[camp->y][camp->x].terrain = TERRAIN_PLAIN;

    for (Unit *u = gs->units; u; u = u->next) {
        if (u->x == camp->x && u->y == camp->y) {
            u->x = -1;
            u->y = -1;
        }
    }

    barbarians_turn(gs);
    ASSERT(gs->barb_unit_count == barbs_before + 1);

    Unit *player = unit_at(gs, gs->cities->x, gs->cities->y, UNIT_OWNER_PLAYER);
    if (!player)
        player = unit_create(gs, UNIT_TYPE_WARRIOR, UNIT_OWNER_PLAYER, gs->cities->x, gs->cities->y);

    Unit *barb = unit_create(gs, UNIT_TYPE_BARB, UNIT_OWNER_BARB, gs->cities->x + 1, gs->cities->y);
    int hp_before = player->hp;
    gs->barb_unit_count++;

    barbarians_turn(gs);
    ASSERT(player->hp < hp_before || unit_by_id(gs, player->id) == NULL);

    free_game(gs);
}

int main(void) {
    printf("Tests :\n");

    printf("chebyshev... "); test_chebyshev(); printf("ok\n");
    printf("seed determinism... "); test_seed_determinism(); printf("ok\n");
    printf("colonise... "); test_colonise(); printf("ok\n");
    printf("city growth... "); test_city_growth(); printf("ok\n");
    printf("city famine... "); test_city_famine(); printf("ok\n");
    printf("production... "); test_production(); printf("ok\n");
    printf("tech... "); test_tech(); printf("ok\n");
    printf("combat... "); test_combat(); printf("ok\n");
    printf("unit move... "); test_unit_move(); printf("ok\n");
    printf("game result... "); test_game_result(); printf("ok\n");
    printf("combat city... "); test_combat_city(); printf("ok\n");
    printf("barbarians... "); test_barbarians(); printf("ok\n");

    printf("\n%d passed, %d failed\n", passes, failures);
    return failures > 0 ? 1 : 0;
}
