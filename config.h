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

#endif /* CONFIG_H */
