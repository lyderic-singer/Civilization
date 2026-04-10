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
    int      unlock_warrior;
    int      unlock_wall;
    int      unlock_library;
    int      unlock_market;
    int      bonus_food_pct;
    int      bonus_prod_pct;
    int      bonus_sci_pct;
    int      bonus_gold_pct;
    int      bonus_pm;
    int      bonus_forest_food;
    int      bonus_food2_pct;
} TechDef;

typedef struct {
    int  acquired[TECH_COUNT];
    int  researching;
    int  science_stored;
} TechState;

/* ═══════════════════════════════════════════════════════════
   UNITS
═══════════════════════════════════════════════════════════ */

typedef enum {
    UNIT_TYPE_SETTLER  = 0,
    UNIT_TYPE_WARRIOR  = 1,
    UNIT_TYPE_BARB     = 2   /* barbarian warrior */
} UnitType;

typedef enum {
    UNIT_OWNER_PLAYER = 0,
    UNIT_OWNER_BARB   = 1
} UnitOwner;

typedef struct Unit {
    int       id;
    UnitType  type;
    UnitOwner owner;
    int       x, y;
    int       hp, hp_max;
    int       attack;
    int       defense;
    int       pm_max;
    int       pm_left;      /* remaining PM this turn */
    int       maint;        /* gold/turn */
    struct Unit *next;      /* linked list */
} Unit;

/* ═══════════════════════════════════════════════════════════
   CITIES
═══════════════════════════════════════════════════════════ */

typedef struct City {
    int   id;
    int   x, y;
    int   population;
    int   food_stock;    /* current food reserve */
    int   prod_stock;    /* production accumulated toward project */
    int   hp, hp_max;
    int   buildings;     /* bitmask of BuildingFlag */
    int   founded_turn;  /* for tiebreaking tile ownership */
    int   project_type;  /* 0=none 1=unit 2=building */
    int   project_id;    /* UnitType or BuildingFlag */
    int   took_damage_this_turn; /* for HP regen logic */
    /* per-turn yields cached after exploitation calc */
    int   yield_food;
    int   yield_prod;
    int   yield_gold;
    int   yield_science;
    struct City *next;
} City;

/* ═══════════════════════════════════════════════════════════
   BARBARIAN CAMP
═══════════════════════════════════════════════════════════ */

typedef struct Camp {
    int id;
    int x, y;
    int alive;
    struct Camp *next;
} Camp;

/* ═══════════════════════════════════════════════════════════
   TILE
═══════════════════════════════════════════════════════════ */

typedef struct {
    TerrainType terrain;
    int         city_id;   /* -1 = none */
    int         camp_id;   /* -1 = none */
} Tile;

/* ═══════════════════════════════════════════════════════════
   MAP
═══════════════════════════════════════════════════════════ */

typedef struct {
    int    width, height;
    Tile **tiles;          /* tiles[y][x] */
} Map;

/* ═══════════════════════════════════════════════════════════
   SEED  (40 hex chars = 160 bits, ETH-address-sized)
═══════════════════════════════════════════════════════════ */

typedef struct {
    uint64_t parts[3];
    char     hex[SEED_HEX_LEN + 1];
} Seed;

/* ═══════════════════════════════════════════════════════════
   GLOBAL GAME STATE
═══════════════════════════════════════════════════════════ */

typedef enum {
    RESULT_ONGOING  = 0,
    RESULT_WIN_SCI  = 1,
    RESULT_WIN_TERR = 2,
    RESULT_LOSS_NOCITY = 3,
    RESULT_LOSS_TURNS  = 4,
    RESULT_LOSS_PROD   = 5
} GameResult;

typedef struct {
    Map        map;
    Unit      *units;
    City      *cities;
    Camp      *camps;
    TechState  tech;

    int        gold;
    int        turn;
    int        turn_max;
    int        n_barbarian_camps_initial;
    int        n_camps_destroyed;
    int        barb_unit_count;

    int        territorial_consecutive_turns;
    int        zero_prod_turns;

    int        next_unit_id;
    int        next_city_id;
    int        next_camp_id;

    Seed       seed;
    GameResult result;
    int        score;

    /* tech effect cache */
    int        bonus_food_pct;
    int        bonus_prod_pct;
    int        bonus_sci_pct;
    int        bonus_gold_pct;
    int        extra_pm;
    int        bonus_forest_food;
    int        warrior_unlocked;
    int        wall_unlocked;
    int        library_unlocked;
    int        market_unlocked;
} GameState;

/* ═══════════════════════════════════════════════════════════
   PROJECT TYPES (for city->project_type)
═══════════════════════════════════════════════════════════ */
#define PROJECT_NONE     0
#define PROJECT_UNIT     1
#define PROJECT_BUILDING 2

#endif /* STRUCTS_H */
