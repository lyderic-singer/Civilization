#ifndef STRUCTS_H
#define STRUCTS_H

#include "../config.h"

/* ════ MAP ════ */

typedef enum {
    TERRAIN_PLAIN = 0,
    TERRAIN_FOREST = 1,
    TERRAIN_MOUNTAIN = 2,
    TERRAIN_WATER = 3,
    TERRAIN_DESERT = 4,
    TERRAIN_TUNDRA = 5,
    TERRAIN_COUNT = 6
} TerrainType;

typedef struct {
    TerrainType terrain;
    int city_id;   /* -1 = none */
    int camp_id;   /* -1 = none */
} Tile;

typedef struct {
    int width, height;
    Tile **tiles;
} Map;

typedef struct {
    int food;
    int prod;
    int gold;
    int science;
    int move_cost;  /* 0 = impassable */
} TerrainInfo;

/* ════ BUILDINGS ════ */

typedef enum {
    BUILDING_NONE = 0,
    BUILDING_GRANARY  = (1 << 0),
    BUILDING_WORKSHOP = (1 << 1),
    BUILDING_LIBRARY = (1 << 2),
    BUILDING_MARKET = (1 << 3),
    BUILDING_BARRACKS = (1 << 4),
    BUILDING_WALL = (1 << 5)
} BuildingFlag;

/* ════ TECHNOLOGIES ════ */

typedef struct {
    int  id;
    const char *name;
    int cost;            /* science points */
    int prereqs[4];      /* prereq tech IDs, -1 = none */
    int prereq_count;
    int unlock_warrior;  /* 1 = unlocks warrior unit */
    int unlock_wall;     /* 1 = unlocks wall building */
    int unlock_library;  /* 1 = unlocks library building */
    int unlock_market;   /* 1 = unlocks market building */
    int bonus_food_pct;  /* % bonus to all city food */
    int bonus_prod_pct;
    int bonus_sci_pct;
    int bonus_gold_pct;
    int bonus_pm;        /* +PM to all terrestrial units */
    int bonus_forest_food; /* +food on forest tiles */
    int bonus_food2_pct; /* stacked irrigation bonus */
} TechDef;

typedef struct {
    int acquired[TECH_COUNT];   /* 0 or 1 */
    int researching;            /* TECH_ID_xxx or TECH_ID_NONE */
    int science_stored;         /* accumulated science toward current research */
} TechState;

/* ════ UNITS ════ */

typedef enum {
    UNIT_TYPE_SETTLER = 0,
    UNIT_TYPE_WARRIOR = 1,
    UNIT_TYPE_BARB = 2   /* barbarian warrior */
} UnitType;

typedef enum {
    UNIT_OWNER_PLAYER = 0,
    UNIT_OWNER_BARB = 1
} UnitOwner;

typedef struct Unit {
    int id;
    UnitType type;
    UnitOwner owner;
    int x, y;
    int hp, hp_max;
    int attack;
    int defense;
    int pm_max;
    int pm_left;      /* remaining PM this turn */
    int maint;        /* gold/turn */
    struct Unit *next;      /* linked list */
} Unit;

/* ════ CITIES ════ */

typedef struct City {
    int id;
    int x, y;
    int population;
    int food_stock;    /* current food reserve */
    int prod_stock;    /* production accumulated toward project */
    int hp, hp_max;
    int buildings;     /* bitmask of BuildingFlag */
    int founded_turn;  /* for tiebreaking tile ownership */
    int project_type;  /* 0=none 1=unit 2=building */
    int project_id;    /* UnitType or BuildingFlag */
    int took_damage_this_turn; /* for HP regen logic */
    /* per-turn yields cached after exploitation calc */
    int yield_food;
    int yield_prod;
    int yield_gold;
    int yield_science;
    struct City *next;
} City;

/* ════ CAMP ════ */

typedef struct Camp {
    int id;
    int x, y;
    int alive;
    struct Camp *next;
} Camp;

typedef struct {
    uint64_t value;
    char hex[SEED_HEX_LEN + 1];
} Seed;

/* ═════ GAME ═════ */

typedef enum {
    RESULT_ONGOING = 0,
    RESULT_WIN_SCI = 1,
    RESULT_WIN_TERR = 2,
    RESULT_LOSS_NOCITY = 3,
    RESULT_LOSS_TURNS = 4,
    RESULT_LOSS_PROD = 5
} GameResult;

typedef struct {
    Map map;
    Unit *units;          /* player + barb units, linked list */
    City *cities;         /* player cities, linked list */
    Camp *camps;          /* barb camps, linked list */
    TechState tech;
    int gold;
    /* counters */
    int        turn;
    int        turn_max;
    int        n_barbarian_camps_initial;
    int        n_camps_destroyed;
    int        barb_unit_count;
    int        territorial_consecutive_turns; /* consecutive turns with 10+ cities */
    int        zero_prod_turns;  /* consecutive turns with total production = 0 */
    /* last unit id assigned */
    int        next_unit_id;
    int        next_city_id;
    int        next_camp_id;
    Seed       seed;
    GameResult result;
    int        score;
    /* tech effect cache (recomputed after each tech acquired) */
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
    char       player_name[32];
} GameState;

#endif