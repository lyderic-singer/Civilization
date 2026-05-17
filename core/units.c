#include "core.h"

const int UNIT_COSTS[3] = {UNIT_SETTLER_COST, UNIT_WARRIOR_COST, UNIT_WARRIOR_COST};
const int UNIT_MAINTS[3] = {UNIT_SETTLER_MAINT, UNIT_WARRIOR_MAINT, UNIT_WARRIOR_MAINT};
const int UNIT_HP_MAXS[3] = {UNIT_SETTLER_HP, UNIT_WARRIOR_HP, UNIT_WARRIOR_HP};
const int UNIT_ATKS[3] = {UNIT_SETTLER_ATK, UNIT_WARRIOR_ATK, UNIT_WARRIOR_ATK};
const int UNIT_DEFS[3] = {UNIT_SETTLER_DEF, UNIT_WARRIOR_DEF, UNIT_WARRIOR_DEF};
const int UNIT_PM_BASES[3] = {UNIT_SETTLER_PM, UNIT_WARRIOR_PM, UNIT_WARRIOR_PM};
const char *UNIT_NAMES[3] = {"Settler", "Warrior", "Barbarian"};

int  unit_cost(UnitType t) { return UNIT_COSTS[t];}
int  unit_maint(UnitType t) { return UNIT_MAINTS[t];}
const char *unit_type_name(UnitType t) { return UNIT_NAMES[t];}

Unit *unit_create(GameState *gs, UnitType type, UnitOwner owner, int x, int y) {
    Unit *u = malloc(sizeof(Unit));
    if (!u) { perror("malloc"); exit(1); }
    u->id = gs->next_unit_id++;
    u->type = type;
    u->owner = owner;
    u->x = x;
    u->y = y;
    u->hp_max = UNIT_HP_MAXS[type];
    u->hp = u->hp_max;
    u->attack = UNIT_ATKS[type];
    u->defense = UNIT_DEFS[type];
    u->pm_max = UNIT_PM_BASES[type] + (owner == UNIT_OWNER_PLAYER ? gs->extra_pm : 0);
    u->pm_left = u->pm_max;
    u->maint = unit_maint(type);
    u->next = gs->units;
    gs->units = u;
    if (owner == UNIT_OWNER_BARB) gs->barb_unit_count++;
    return u;
}

void unit_destroy(GameState *gs, int unit_id) {
    Unit **pp = &gs->units;
    while (*pp) {
        if ((*pp)->id == unit_id) {
            Unit *dead = *pp;
            *pp = dead->next;
            if (dead->owner == UNIT_OWNER_BARB) gs->barb_unit_count--;
            free(dead);
            return;
        }
        pp = &(*pp)->next;
    }
}

Unit *unit_at(GameState *gs, int x, int y, UnitOwner owner) {
    for (Unit *u = gs->units; u; u = u->next)
        if (u->x == x && u->y == y && u->owner == owner)
            return u;
    return NULL;
}

Unit *unit_any_at(GameState *gs, int x, int y) {
    for (Unit *u = gs->units; u; u = u->next)
        if (u->x == x && u->y == y)
            return u;
    return NULL;
}

Unit *unit_by_id(GameState *gs, int id) {
    for (Unit *u = gs->units; u; u = u->next)
        if (u->id == id)
            return u;
    return NULL;
}

int units_count(GameState *gs, UnitOwner owner) {
    int n = 0;
    for (Unit *u = gs->units; u; u = u->next)
        if (u->owner == owner) n++;
    return n;
}

int units_count_all(GameState *gs) {
    int n = 0;
    for (Unit *u = gs->units; u; u = u->next) n++;
    return n;
}

int tile_move_cost(GameState *gs, int x, int y) {
    if (x < 0 || x >= gs->map.width || y < 0 || y >= gs->map.height)
        return 0;
    return MOVE_COSTS[gs->map.tiles[y][x].terrain];
}

int unit_move(GameState *gs, Unit *u, int nx, int ny) {
    if (nx < 0 || nx >= gs->map.width || ny < 0 || ny >= gs->map.height)
        return 0;
    int cost = tile_move_cost(gs, nx, ny);
    if (cost == 0) return 0; /* impassable (water) */
    if (u->pm_left < cost) return 0; /* not enough PM */
    Unit *enemy = unit_any_at(gs, nx, ny);
    if (enemy) {
        if (enemy->owner == u->owner) return 0; /* friendly blocking */
        int killed = unit_attack_unit(gs, u, enemy);
        if (!killed) return 0;
        if (!unit_by_id(gs, u->id)) return 0;
    }
    if (u->owner == UNIT_OWNER_PLAYER) {
        int camp_id = gs->map.tiles[ny][nx].camp_id;
        if (camp_id != -1) {
            for (Camp *c = gs->camps; c; c = c->next) {
                if (c->id == camp_id && c->alive) {
                    c->alive = 0;
                    gs->map.tiles[ny][nx].camp_id = -1;
                    gs->n_camps_destroyed++;
                    gs->gold += BARB_GOLD_REWARD_FACTOR * gs->n_camps_destroyed;
                    printf("[Camp detruit] +%d or (total camps: %d)\n",
                           BARB_GOLD_REWARD_FACTOR * gs->n_camps_destroyed,
                           gs->n_camps_destroyed);
                    break;
    }}}}
    u->pm_left -= cost;
    u->x = nx;
    u->y = ny;
    return 1;
}

void units_reset_pm(GameState *gs, UnitOwner owner) {
    for (Unit *u = gs->units; u; u = u->next)
        if (u->owner == owner)
            u->pm_left = u->pm_max;
}

int unit_attack_unit(GameState *gs, Unit *attacker, Unit *defender) {
    int dmg_to_def  = attacker->attack - defender->defense;
    if (dmg_to_def < 1) dmg_to_def = 1;
    int dmg_to_atk  = defender->attack - attacker->defense;
    if (dmg_to_atk < 0) dmg_to_atk = 0;
    defender->hp -= dmg_to_def;
    attacker->hp -= dmg_to_atk;
    printf("[Combat] %s(%d) vs %s(%d) ==> -%d/-%d PV\n",
           unit_type_name(attacker->type), attacker->id,
           unit_type_name(defender->type), defender->id,
           dmg_to_def, dmg_to_atk);
    int def_dead = 0;
    if (defender->hp <= 0) {
        printf("[Unite detruite] %s id=%d\n", unit_type_name(defender->type), defender->id);
        unit_destroy(gs, defender->id);
        def_dead = 1;
    }
    if (attacker->hp <= 0) {
        printf("[Unite detruite] %s id=%d\n", unit_type_name(attacker->type), attacker->id);
        unit_destroy(gs, attacker->id);
    }
    return def_dead;
}

int unit_attack_city(GameState *gs, Unit *attacker, City *city) {
    int has_wall = (city->buildings & BUILDING_WALL) ? 1 : 0;
    int city_def  = has_wall ? CITY_DEFENSE_WALL : CITY_DEFENSE_BASE;
    int city_atk  = city_def; /* city attacks and defends with same value */
    int dmg_to_city   = attacker->attack - city_def;
    if (dmg_to_city < 1) dmg_to_city = 1;
    int dmg_to_unit   = city_atk - attacker->defense;
    if (dmg_to_unit < 0) dmg_to_unit = 0;
    city->hp -= dmg_to_city;
    city->took_damage_this_turn = 1;
    attacker->hp -= dmg_to_unit;
    printf("[Siege] %s attaque ville id=%d ==> ville -%d PV, unite -%d PV\n", unit_type_name(attacker->type), city->id, dmg_to_city, dmg_to_unit);
    if (attacker->hp <= 0) {
        unit_destroy(gs, attacker->id);
    }
    if (city->hp <= 0) {
        printf("[Ville rasee] id=%d\n", city->id);
        city_raze(gs, city->id);
        return 1;
    }
    return 0;
}

void units_apply_pm_bonus(GameState *gs) {
    for (Unit *u = gs->units; u; u = u->next) {
        if (u->owner == UNIT_OWNER_PLAYER) {
            u->pm_max = UNIT_PM_BASES[u->type] + gs->extra_pm;
            if (u->pm_left > u->pm_max) u->pm_left = u->pm_max;
    }}
}

void unit_goto(GameState *gs, Unit *u, int tx, int ty) {
    while (u->pm_left > 0) {
        if (u->x == tx && u->y == ty) break;
        int best_dx = 0, best_dy = 0;
        int best_d = chebyshev(u->x, u->y, tx, ty);
        int best_cost = 999;
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (dx == 0 && dy == 0) continue;
                int nx = u->x + dx, ny = u->y + dy;
                if (nx < 0 || nx >= gs->map.width || ny < 0 || ny >= gs->map.height) continue;
                TerrainType t = gs->map.tiles[ny][nx].terrain;
                int cost = MOVE_COSTS[t];
                if (cost == 0 || cost > u->pm_left) continue;
                Unit *occ = unit_at(gs, nx, ny, UNIT_OWNER_PLAYER);
                if (occ && occ->id != u->id) continue;
                int d = chebyshev(nx, ny, tx, ty);
                if (d < best_d || (d == best_d && cost < best_cost)) {
                    best_d = d; best_cost = cost;
                    best_dx = dx; best_dy = dy;
        }}}
        if (best_dx == 0 && best_dy == 0) break;
        int nx = u->x + best_dx, ny = u->y + best_dy;
        if (!unit_move(gs, u, nx, ny)) break;
        if (!unit_by_id(gs, u->id)) break;
    }
}
