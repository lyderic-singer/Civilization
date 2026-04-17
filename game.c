#include "game.h"
#include "gen.h"
#include "units.h"
#include "cities.h"
#include "barbarians.h"
#include "tech.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ═══════════════════════════════════════════════════════════
   TERRAIN DISPLAY
═══════════════════════════════════════════════════════════ */

static char terrain_char(TerrainType t) {
    switch (t) {
        case TERRAIN_PLAIN:    return '.';
        case TERRAIN_FOREST:   return 'T';
        case TERRAIN_MOUNTAIN: return 'M';
        case TERRAIN_WATER:    return '~';
        case TERRAIN_DESERT:   return 'D';
        case TERRAIN_TUNDRA:   return '_';
        default:               return '?';
    }
}

/* ═══════════════════════════════════════════════════════════
   INIT / FREE
═══════════════════════════════════════════════════════════ */

GameState *game_init(int width, int height, int turns, int n_barb_camps, Seed seed) {
    GameState *gs = calloc(1, sizeof(GameState));
    if (!gs) { perror("calloc"); exit(1); }

    gs->map.width  = width;
    gs->map.height = height;
    gs->turn       = 1;
    gs->turn_max   = turns;
    gs->n_barbarian_camps_initial = n_barb_camps;
    gs->n_camps_destroyed = 0;
    gs->barb_unit_count   = 0;
    gs->gold              = 0;
    gs->territorial_consecutive_turns = 0;
    gs->zero_prod_turns   = 0;
    gs->result            = RESULT_ONGOING;
    gs->next_unit_id      = 0;
    gs->next_city_id      = 0;
    gs->next_camp_id      = 0;
    gs->seed              = seed;

    tech_init(&gs->tech);
    tech_apply_effects(gs);

    gen_map(gs);

    printf("\n=== Early Civilization ===\n");
    printf("Seed: 0x%s\n", gs->seed.hex);
    printf("Carte: %dx%d | Tours max: %d | Camps barbares: %d\n\n",
           width, height, turns, n_barb_camps);

    return gs;
}

static void free_units(Unit *u) {
    while (u) { Unit *n = u->next; free(u); u = n; }
}
static void free_cities(City *c) {
    while (c) { City *n = c->next; free(c); c = n; }
}
static void free_camps(Camp *c) {
    while (c) { Camp *n = c->next; free(c); c = n; }
}
static void free_map(Map *m) {
    if (!m->tiles) return;
    for (int y = 0; y < m->height; y++) free(m->tiles[y]);
    free(m->tiles);
    m->tiles = NULL;
}

void game_free(GameState *gs) {
    free_units(gs->units);
    free_cities(gs->cities);
    free_camps(gs->camps);
    free_map(&gs->map);
    free(gs);
}
