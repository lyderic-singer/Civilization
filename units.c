#include "units.h"
#include "cities.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════
   STAT TABLES
═══════════════════════════════════════════════════════════ */

static const int UNIT_COSTS[3]    = {UNIT_SETTLER_COST,    UNIT_WARRIOR_COST,    UNIT_WARRIOR_COST};
static const int UNIT_MAINTS[3]   = {UNIT_SETTLER_MAINT,   UNIT_WARRIOR_MAINT,   UNIT_WARRIOR_MAINT};
static const int UNIT_HP_MAXS[3]  = {UNIT_SETTLER_HP,      UNIT_WARRIOR_HP,      UNIT_WARRIOR_HP};
static const int UNIT_ATKS[3]     = {UNIT_SETTLER_ATK,     UNIT_WARRIOR_ATK,     UNIT_WARRIOR_ATK};
static const int UNIT_DEFS[3]     = {UNIT_SETTLER_DEF,     UNIT_WARRIOR_DEF,     UNIT_WARRIOR_DEF};
static const int UNIT_PM_BASES[3] = {UNIT_SETTLER_PM,      UNIT_WARRIOR_PM,      UNIT_WARRIOR_PM};
static const char *UNIT_NAMES[3]  = {"Settler", "Warrior", "Barbarian"};

int  unit_cost(UnitType t)     { return UNIT_COSTS[t]; }
int  unit_maint(UnitType t)    { return UNIT_MAINTS[t]; }
int  unit_hp_max(UnitType t)   { return UNIT_HP_MAXS[t]; }
int  unit_atk(UnitType t)      { return UNIT_ATKS[t]; }
int  unit_def(UnitType t)      { return UNIT_DEFS[t]; }
int  unit_pm_base(UnitType t)  { return UNIT_PM_BASES[t]; }
const char *unit_type_name(UnitType t) { return UNIT_NAMES[t]; }

/* ═══════════════════════════════════════════════════════════
   LIFECYCLE
═══════════════════════════════════════════════════════════ */

Unit *unit_create(GameState *gs, UnitType type, UnitOwner owner, int x, int y) {
    Unit *u = malloc(sizeof(Unit));
    if (!u) { perror("malloc"); exit(1); }
    u->id      = gs->next_unit_id++;
    u->type    = type;
    u->owner   = owner;
    u->x       = x;
    u->y       = y;
    u->hp_max  = unit_hp_max(type);
    u->hp      = u->hp_max;
    u->attack  = unit_atk(type);
    u->defense = unit_def(type);
    u->pm_max  = unit_pm_base(type) + (owner == UNIT_OWNER_PLAYER ? gs->extra_pm : 0);
    u->pm_left = u->pm_max;
    u->maint   = unit_maint(type);
    u->next    = gs->units;
    gs->units  = u;
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
