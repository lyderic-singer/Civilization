#ifndef CONFIG_H
#define CONFIG_H

/* ==== Default parameters ==== */
#define DEFAULT_WIDTH       50
#define DEFAULT_HEIGHT      30
#define DEFAULT_TURNS       200
#define DEFAULT_BARBARIANS  3
#define DEFAULT_MODE        "cli"

/* ==== Terrain movement costs (0 = impassable) ==== */
#define TERRAIN_PLAIN_COST    1
#define TERRAIN_FOREST_COST   2
#define TERRAIN_MOUNTAIN_COST 3
#define TERRAIN_WATER_COST    0   /* impassable */
#define TERRAIN_DESERT_COST   1
#define TERRAIN_TUNDRA_COST   1

/* ==== Terrain resource bonuses [food, prod, gold, science] ==== */
#define TERRAIN_PLAIN_FOOD    2
#define TERRAIN_PLAIN_PROD    1
#define TERRAIN_PLAIN_GOLD    0
#define TERRAIN_PLAIN_SCI     0

#define TERRAIN_FOREST_FOOD   1
#define TERRAIN_FOREST_PROD   2
#define TERRAIN_FOREST_GOLD   0
#define TERRAIN_FOREST_SCI    0

#define TERRAIN_MOUNTAIN_FOOD 0
#define TERRAIN_MOUNTAIN_PROD 3
#define TERRAIN_MOUNTAIN_GOLD 0
#define TERRAIN_MOUNTAIN_SCI  1

#define TERRAIN_WATER_FOOD    1
#define TERRAIN_WATER_PROD    0
#define TERRAIN_WATER_GOLD    1
#define TERRAIN_WATER_SCI     0

#define TERRAIN_DESERT_FOOD   0
#define TERRAIN_DESERT_PROD   0
#define TERRAIN_DESERT_GOLD   1
#define TERRAIN_DESERT_SCI    0

#define TERRAIN_TUNDRA_FOOD   1
#define TERRAIN_TUNDRA_PROD   1
#define TERRAIN_TUNDRA_GOLD   0
#define TERRAIN_TUNDRA_SCI    0

/* ==== City rules ==== */
#define CITY_MIN_DISTANCE       4   /* Chebyshev distance between cities */
#define CITY_START_POPULATION   1
#define CITY_FOOD_CONSUMPTION_FACTOR 2   /* 2*pop per turn */
#define CITY_GROWTH_BASE        20
#define CITY_GROWTH_PER_POP     10  /* threshold = 20 + 10*pop */
#define CITY_HP_PER_POP         10
#define CITY_DEFENSE_BASE       20
#define CITY_DEFENSE_WALL       30
#define CITY_HEAL_DIVISOR       2   /* restore HP/2 per turn if no damage */

/* ==== Exploitation radius by population ==== */
#define CITY_RADIUS_SMALL       1   /* pop 1-9 */
#define CITY_RADIUS_MEDIUM      2   /* pop 10-25 */
#define CITY_RADIUS_LARGE       3   /* pop 26+ */
#define CITY_POP_MEDIUM_THRESH  10
#define CITY_POP_LARGE_THRESH   26

/* ==== Buildings ==== */
#define BUILDING_GRANARY_COST       30
#define BUILDING_GRANARY_MAINT      1
#define BUILDING_GRANARY_FOOD       3
#define BUILDING_GRANARY_GROWTH_DIV 1.5f

#define BUILDING_WORKSHOP_COST      40
#define BUILDING_WORKSHOP_MAINT     1
#define BUILDING_WORKSHOP_PROD      3

#define BUILDING_LIBRARY_COST       50
#define BUILDING_LIBRARY_MAINT      1
#define BUILDING_LIBRARY_SCI        4

#define BUILDING_MARKET_COST        40
#define BUILDING_MARKET_MAINT       1
#define BUILDING_MARKET_GOLD        3

#define BUILDING_BARRACKS_COST      60
#define BUILDING_BARRACKS_MAINT     2

#define BUILDING_WALL_COST          80
#define BUILDING_WALL_MAINT         2

/* ==== Units ==== */
#define UNIT_SETTLER_COST    50
#define UNIT_SETTLER_MAINT   0
#define UNIT_SETTLER_HP      10
#define UNIT_SETTLER_ATK     0
#define UNIT_SETTLER_DEF     1
#define UNIT_SETTLER_PM      2

#define UNIT_WARRIOR_COST    40
#define UNIT_WARRIOR_MAINT   1
#define UNIT_WARRIOR_HP      15
#define UNIT_WARRIOR_ATK     3
#define UNIT_WARRIOR_DEF     2
#define UNIT_WARRIOR_PM      3

/* ==== Technologies ==== */
#define TECH_COUNT 9

#define TECH_ID_HUNTING    0
#define TECH_ID_FARMING    1
#define TECH_ID_IRRIGATION 2
#define TECH_ID_CRAFT      3
#define TECH_ID_MASONRY    4
#define TECH_ID_WRITING    5
#define TECH_ID_COMMERCE   6
#define TECH_ID_RIDING     7
#define TECH_ID_NONE       -1

#define TECH_COST_HUNTING    50
#define TECH_COST_FARMING    60
#define TECH_COST_IRRIGATION 90
#define TECH_COST_CRAFT      70
#define TECH_COST_MASONRY    100
#define TECH_COST_WRITING    80
#define TECH_COST_COMMERCE   90
#define TECH_COST_RIDING     100

/* ==== Barbarians ==== */
#define BARB_MIN_DIST_FROM_START 5   /* Chebyshev distance */
#define BARB_UNIT_CAP_FACTOR     3   /* max = N_camps * 3 */
#define BARB_GOLD_REWARD_FACTOR  5   /* reward = 5 * n_destroyed */

/* ==== Gold / bankruptcy ==== */
#define GOLD_BANKRUPT_PROD_HALVE_TURNS 5

/* ==== Victory conditions ==== */
#define VICTORY_TERRITORIAL_CITIES  10
#define VICTORY_TERRITORIAL_TURNS   5

/* ==== Score formula weights ==== */
#define SCORE_TECH_WEIGHT     100
#define SCORE_POP_WEIGHT      20
#define SCORE_CITY_WEIGHT     50
#define SCORE_TURN_BONUS      10

/* ==== Seed hash display width (characters = 40 hex digits, like ETH addr) ==== */
#define SEED_HEX_LEN 40

#endif /* CONFIG_H */
