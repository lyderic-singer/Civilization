#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdint.h>
#include "config.h"

/* ═══════════════════════════════════════════════════════════
   TERRAIN
═══════════════════════════════════════════════════════════ */

typedef enum {
    TERRAIN_PLAIN    = 0,
    TERRAIN_FOREST   = 1,
    TERRAIN_MOUNTAIN = 2,
    TERRAIN_WATER    = 3,
    TERRAIN_DESERT   = 4,
    TERRAIN_TUNDRA   = 5,
    TERRAIN_COUNT    = 6
} TerrainType;

typedef struct {
    int food;
    int prod;
    int gold;
    int science;
    int move_cost;  /* 0 = impassable */
} TerrainInfo;

/* ═══════════════════════════════════════════════════════════
   BUILDINGS
═══════════════════════════════════════════════════════════ */

typedef enum {
    BUILDING_NONE     = 0,
    BUILDING_GRANARY  = (1 << 0),
    BUILDING_WORKSHOP = (1 << 1),
    BUILDING_LIBRARY  = (1 << 2),
    BUILDING_MARKET   = (1 << 3),
    BUILDING_BARRACKS = (1 << 4),
    BUILDING_WALL     = (1 << 5)
} BuildingFlag;

/* ═══════════════════════════════════════════════════════════
   TECHNOLOGIES
═══════════════════════════════════════════════════════════ */

typedef struct {
    int      id;
    const char *name;
    int      cost;            /* science points */
    int      prereqs[4];      /* prereq tech IDs, -1 = none */
    int      prereq_count;
    /* effects stored as deltas; game.c applies them */
    int      unlock_warrior;  /* 1 = unlocks warrior unit */
    int      unlock_wall;     /* 1 = unlocks wall building */
    int      unlock_library;  /* 1 = unlocks library building */
    int      unlock_market;   /* 1 = unlocks market building */
    int      bonus_food_pct;  /* % bonus to all city food */
    int      bonus_prod_pct;
    int      bonus_sci_pct;
    int      bonus_gold_pct;
    int      bonus_pm;        /* +PM to all terrestrial units */
    int      bonus_forest_food; /* +food on forest tiles */
    int      bonus_food2_pct; /* stacked irrigation bonus */
} TechDef;

typedef struct {
    int  acquired[TECH_COUNT];   /* 0 or 1 */
    int  researching;            /* TECH_ID_xxx or TECH_ID_NONE */
    int  science_stored;         /* accumulated science toward current research */
} TechState;

#endif /* STRUCTS_H */
