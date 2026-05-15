#ifndef SDL_UI_H
#define SDL_UI_H

#include "../core/core.h"
#include <sys/types.h>

#define TILE_SIZE   16

typedef struct { u_int8_t r, g, b; } Color;

static const Color TERRAIN_COLORS[TERRAIN_COUNT] = {
    [TERRAIN_PLAIN]    = { 144, 200, 100 },
    [TERRAIN_FOREST]   = {  34, 100,  34 },
    [TERRAIN_MOUNTAIN] = { 150, 130, 100 },
    [TERRAIN_WATER]    = {  64, 120, 200 },
    [TERRAIN_DESERT]   = { 230, 200, 100 },
    [TERRAIN_TUNDRA]   = { 200, 220, 230 },
};

void sdl_run(GameState *gs);

#endif
