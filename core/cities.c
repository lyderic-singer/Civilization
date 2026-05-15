#include "core.h"

const BuildingInfo BUILDINGS[] = {
    {BUILDING_GRANARY,  "Grenier",      BUILDING_GRANARY_COST,   BUILDING_GRANARY_MAINT},
    {BUILDING_WORKSHOP, "Atelier",      BUILDING_WORKSHOP_COST,  BUILDING_WORKSHOP_MAINT},
    {BUILDING_LIBRARY,  "Bibliotheque", BUILDING_LIBRARY_COST,   BUILDING_LIBRARY_MAINT},
    {BUILDING_MARKET,   "Marche",       BUILDING_MARKET_COST,    BUILDING_MARKET_MAINT},
    {BUILDING_BARRACKS, "Caserne",      BUILDING_BARRACKS_COST,  BUILDING_BARRACKS_MAINT},
    {BUILDING_WALL,     "Muraille",     BUILDING_WALL_COST,      BUILDING_WALL_MAINT},
};

const int T_FOOD[TERRAIN_COUNT] = {TERRAIN_PLAIN_FOOD, TERRAIN_FOREST_FOOD, TERRAIN_MOUNTAIN_FOOD, TERRAIN_WATER_FOOD, TERRAIN_DESERT_FOOD, TERRAIN_TUNDRA_FOOD};
const int T_PROD[TERRAIN_COUNT] = {TERRAIN_PLAIN_PROD, TERRAIN_FOREST_PROD, TERRAIN_MOUNTAIN_PROD, TERRAIN_WATER_PROD, TERRAIN_DESERT_PROD, TERRAIN_TUNDRA_PROD};
const int T_GOLD[TERRAIN_COUNT] = {TERRAIN_PLAIN_GOLD, TERRAIN_FOREST_GOLD, TERRAIN_MOUNTAIN_GOLD, TERRAIN_WATER_GOLD, TERRAIN_DESERT_GOLD, TERRAIN_TUNDRA_GOLD};
const int T_SCI[TERRAIN_COUNT] = {TERRAIN_PLAIN_SCI, TERRAIN_FOREST_SCI, TERRAIN_MOUNTAIN_SCI, TERRAIN_WATER_SCI, TERRAIN_DESERT_SCI, TERRAIN_TUNDRA_SCI};

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
    c->id = gs->next_city_id++;
    c->x = x;
    c->y = y;
    c->population = CITY_START_POPULATION;
    c->food_stock = 0;
    c->prod_stock = 0;
    c->buildings = BUILDING_NONE;
    c->founded_turn = founded_turn;
    c->project_type = PROJECT_NONE;
    c->project_id = 0;
    c->took_damage_this_turn = 0;
    c->yield_food = 0;
    c->yield_prod = 0;
    c->yield_gold = 0;
    c->yield_science = 0;
    c->next = gs->cities;
    gs->cities = c;
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

City *tile_owner(GameState *gs, int x, int y) {
    City *owner    = NULL;
    int   best_dist = 999999;
    int   best_turn = 999999;
    for (City *c = gs->cities; c; c = c->next) {
        int d = chebyshev(x, y, c->x, c->y);
        int r = city_exploit_radius(c->population);
        if (d > r) continue;
        if (d < best_dist || (d == best_dist && c->founded_turn < best_turn)) {
            best_dist = d;
            best_turn = c->founded_turn;
            owner = c;
        }
    }
    return owner;
}

void cities_accumulate_terrain(GameState *gs) {
    int W = gs->map.width;
    int H = gs->map.height;
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            City *owner = tile_owner(gs, x, y);
            if (!owner) continue;
            TerrainType t = gs->map.tiles[y][x].terrain;
            int food = T_FOOD[t] + (t == TERRAIN_FOREST ? gs->bonus_forest_food : 0);
            owner->yield_food += food;
            owner->yield_prod += T_PROD[t];
            owner->yield_gold += T_GOLD[t];
            owner->yield_science += T_SCI[t];
    }}
}

void city_apply_bonuses(GameState *gs, City *c) {
    if (c->buildings & BUILDING_GRANARY) c->yield_food += BUILDING_GRANARY_FOOD;
    if (c->buildings & BUILDING_WORKSHOP) c->yield_prod += BUILDING_WORKSHOP_PROD;
    if (c->buildings & BUILDING_LIBRARY) c->yield_science += BUILDING_LIBRARY_SCI;
    if (c->buildings & BUILDING_MARKET) c->yield_gold += BUILDING_MARKET_GOLD;
    c->yield_food += c->yield_food * gs->bonus_food_pct / 100;
    c->yield_prod += c->yield_prod * gs->bonus_prod_pct / 100;
    c->yield_science += c->yield_science * gs->bonus_sci_pct  / 100;
    c->yield_gold += c->yield_gold * gs->bonus_gold_pct / 100;
}

void cities_compute_yields(GameState *gs) {
    for (City *c = gs->cities; c; c = c->next) {
        c->yield_food = c->yield_prod = c->yield_gold = c->yield_science = 0;
    }
    cities_accumulate_terrain(gs);
    for (City *c = gs->cities; c; c = c->next) {
        city_apply_bonuses(gs, c);
    }
}

void city_set_project(City *c, int project_type, int project_id) {
    c->project_type = project_type;
    c->project_id = project_id;
    c->prod_stock = 0;
}

void city_cancel_project(City *c) {
    c->project_type = PROJECT_NONE;
    c->project_id = 0;
}

int city_project_cost(GameState *gs, City *c) {
    (void)gs;
    if (c->project_type == PROJECT_BUILDING)
        return building_cost(c->project_id);
    if (c->project_type == PROJECT_UNIT)
        return unit_cost((UnitType)c->project_id);
    return 0;
}

int city_can_build(GameState *gs, City *c, int building_flag) {
    if (c->buildings & building_flag) return 0; /* already built */
    switch (building_flag) {
        case BUILDING_GRANARY: return 1;
        case BUILDING_WORKSHOP: return 1;
        case BUILDING_BARRACKS: return 1;
        case BUILDING_LIBRARY: return gs->library_unlocked;
        case BUILDING_MARKET: return gs->market_unlocked;
        case BUILDING_WALL: return gs->wall_unlocked;
        default: return 0;
    }
}

void cities_produce(GameState *gs) {
    for (City *c = gs->cities; c; c = c->next) {
        if (c->project_type == PROJECT_NONE) continue;
        c->prod_stock += c->yield_prod;
        int needed = city_project_cost(gs, c);
        if (c->prod_stock < needed) continue;
        c->prod_stock = 0;
        if (c->project_type == PROJECT_BUILDING) {
            int flag = c->project_id;
            c->buildings |= flag;
            city_recompute_hp(c); /* wall doubles HP */
            printf("[Ville %d] Construction complete: %s\n", c->id, building_name(flag));
        } else if (c->project_type == PROJECT_UNIT) {
            UnitType ut = (UnitType)c->project_id;
            if (ut == UNIT_TYPE_WARRIOR && !(c->buildings & BUILDING_BARRACKS)) {
                printf("[Ville %d] Caserne requise pour produire un Guerrier\n", c->id);
                city_cancel_project(c);
                continue;
            }
            Unit *u = unit_create(gs, ut, UNIT_OWNER_PLAYER, c->x, c->y);
            printf("[Ville %d] Unite creee: %s id=%d\n", c->id, unit_type_name(ut), u->id);
        }
        city_cancel_project(c);
    }
}

void cities_grow(GameState *gs) {
    City *next_city;
    for (City *c = gs->cities; c; c = next_city) {
        next_city = c->next; /* save before possible raze */
        int consumption = CITY_FOOD_CONSUMPTION_FACTOR * c->population;
        int net_food = c->yield_food - consumption;
        c->food_stock += net_food;
        int threshold = CITY_GROWTH_BASE + CITY_GROWTH_PER_POP * c->population;
        if (c->buildings & BUILDING_GRANARY)
            threshold = (int)(threshold / BUILDING_GRANARY_GROWTH_DIV);
        if (c->food_stock >= threshold) {
            c->food_stock = 0;
            c->population++;
            city_recompute_hp(c);
            printf("[Ville %d] Croissance! Population: %d\n", c->id, c->population);
        } else if (c->food_stock < 0) {
            c->food_stock = 0;
            c->population--;
            printf("[Ville %d] Famine! Population: %d\n", c->id, c->population);
            if (c->population <= 0) {
                printf("[Ville %d] Abandonnee (famine)\n", c->id);
                city_raze(gs, c->id);
            } else {
                city_recompute_hp(c);
    }}}
}

void cities_heal(GameState *gs) {
    for (City *c = gs->cities; c; c = c->next) {
        if (!c->took_damage_this_turn && c->hp < c->hp_max) {
            c->hp += c->hp_max / CITY_HEAL_DIVISOR;
            if (c->hp > c->hp_max) c->hp = c->hp_max;
    }}
}

void cities_reset_damage_flag(GameState *gs) {
    for (City *c = gs->cities; c; c = c->next) {
        c->took_damage_this_turn = 0;
    }
}

int cities_total_building_maint(GameState *gs) {
    int total = 0;
    for (City *c = gs->cities; c; c = c->next) {
        for (int i = 0; i < BUILDING_COUNT; i++) {
            if (c->buildings & BUILDINGS[i].flag)
                total += BUILDINGS[i].maint;
    }}
    return total;
}
