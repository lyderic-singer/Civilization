#ifndef SDL_UI_H
#define SDL_UI_H

#include "../core/core.h"
#include "../core/turn.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdint.h>

#define SCREEN_W 1280
#define SCREEN_H 720
#define TARGET_TILES_X 40
#define TILE_SIZE (SCREEN_W / TARGET_TILES_X)
#define MARGIN_SIZE (TILE_SIZE / 6)
#define FONT_SIZE 24
#define LINE_H (FONT_SIZE + 8)
#define PANEL_PAD 10
#define PANEL_LINES 3
#define PANEL_H (PANEL_PAD + PANEL_LINES * LINE_H + PANEL_PAD)
#define FONT_PATH "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
#define GAP 20


void sdl_run(GameState *gs);

#endif
