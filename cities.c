#include "cities.h"
#include "units.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ═══════════════════════════════════════════════════════════
   BUILDING TABLES
═══════════════════════════════════════════════════════════ */

typedef struct {
    int         flag;
    const char *name;
    int         cost;
    int         maint;
} BuildingInfo;

static const BuildingInfo BUILDINGS[] = {
    {BUILDING_GRANARY,  "Grenier",      BUILDING_GRANARY_COST,   BUILDING_GRANARY_MAINT},
    {BUILDING_WORKSHOP, "Atelier",      BUILDING_WORKSHOP_COST,  BUILDING_WORKSHOP_MAINT},
    {BUILDING_LIBRARY,  "Bibliotheque", BUILDING_LIBRARY_COST,   BUILDING_LIBRARY_MAINT},
    {BUILDING_MARKET,   "Marche",       BUILDING_MARKET_COST,    BUILDING_MARKET_MAINT},
    {BUILDING_BARRACKS, "Caserne",      BUILDING_BARRACKS_COST,  BUILDING_BARRACKS_MAINT},
    {BUILDING_WALL,     "Muraille",     BUILDING_WALL_COST,      BUILDING_WALL_MAINT},
};
static const int BUILDING_COUNT = 6;

int building_cost(int flag) {
    for (int i = 0; i < BUILDING_COUNT; i++)
        if (BUILDINGS[i].flag == flag) return BUILDINGS[i].cost;
    return 0;
}
int building_maint(int flag) {
    for (int i = 0; i < BUILDING_COUNT; i++)
        if (BUILDINGS[i].flag == flag) return BUILDINGS[i].maint;
    return 0;
}
const char *building_name(int flag) {
    for (int i = 0; i < BUILDING_COUNT; i++)
        if (BUILDINGS[i].flag == flag) return BUILDINGS[i].name;
    return "?";
}

/* ═══════════════════════════════════════════════════════════
   LIFECYCLE
═══════════════════════════════════════════════════════════ */

int city_exploit_radius(int population) {
    if (population >= CITY_POP_LARGE_THRESH)  return CITY_RADIUS_LARGE;
    if (population >= CITY_POP_MEDIUM_THRESH) return CITY_RADIUS_MEDIUM;
    return CITY_RADIUS_SMALL;
}

void city_recompute_hp(City *c) {
    int base_max = c->population * CITY_HP_PER_POP;
    if (c->buildings & BUILDING_WALL) base_max *= 2;
    c->hp_max = base_max;
    if (c->hp > c->hp_max) c->hp = c->hp_max;
    if (c->hp < 0) c->hp = 0;
}

City *city_create(GameState *gs, int x, int y, int founded_turn) {
    City *c = malloc(sizeof(City));
    if (!c) { perror("malloc"); exit(1); }
    c->id            = gs->next_city_id++;
    c->x             = x;
    c->y             = y;
    c->population    = CITY_START_POPULATION;
    c->food_stock    = 0;
    c->prod_stock    = 0;
    c->buildings     = BUILDING_NONE;
    c->founded_turn  = founded_turn;
    c->project_type  = PROJECT_NONE;
    c->project_id    = 0;
    c->took_damage_this_turn = 0;
    c->yield_food    = 0;
    c->yield_prod    = 0;
    c->yield_gold    = 0;
    c->yield_science = 0;
    c->next          = gs->cities;
    gs->cities       = c;
    gs->map.tiles[y][x].city_id = c->id;
    city_recompute_hp(c);
    return c;
}

void city_raze(GameState *gs, int city_id) {
    City **pp = &gs->cities;
    while (*pp) {
        if ((*pp)->id == city_id) {
            City *dead = *pp;
            *pp = dead->next;
            gs->map.tiles[dead->y][dead->x].city_id = -1;
            free(dead);
            return;
        }
        pp = &(*pp)->next;
    }
}

City *city_by_id(GameState *gs, int id) {
    for (City *c = gs->cities; c; c = c->next)
        if (c->id == id) return c;
    return NULL;
}

City *city_at(GameState *gs, int x, int y) {
    for (City *c = gs->cities; c; c = c->next)
        if (c->x == x && c->y == y) return c;
    return NULL;
}

int cities_count(GameState *gs) {
    int n = 0;
    for (City *c = gs->cities; c; c = c->next) n++;
    return n;
}

int city_total_pop(GameState *gs) {
    int n = 0;
    for (City *c = gs->cities; c; c = c->next) n += c->population;
    return n;
}

/* ═══════════════════════════════════════════════════════════
   CHEBYSHEV  (local copy to avoid circular include)
═══════════════════════════════════════════════════════════ */

static int cheb(int ax, int ay, int bx, int by) {
    int dx = ax - bx; if (dx < 0) dx = -dx;
    int dy = ay - by; if (dy < 0) dy = -dy;
    return dx > dy ? dx : dy;
}
