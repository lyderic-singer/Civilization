#include "sdl_ui.h"
#include <SDL2/SDL.h>


static void draw_tile(SDL_Renderer *r, int x, int y, TerrainType t) {
    Color c = TERRAIN_COLORS[t];
    SDL_Rect rect = { x, y, TILE_SIZE, TILE_SIZE };

    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, 255);
    SDL_RenderFillRect(r, &rect);

    /* bordure */
    SDL_SetRenderDrawColor(r, c.r / 2, c.g / 2, c.b / 2, 255);
    SDL_RenderDrawRect(r, &rect);
}

static void draw_map(SDL_Renderer *r, const GameState *gs) {
    for (int y = 0; y < gs->map.height; y++)
        for (int x = 0; x < gs->map.width; x++)
            draw_tile(r, x * TILE_SIZE, y * TILE_SIZE,
                      gs->map.tiles[y][x].terrain);
}

static void draw_entity(SDL_Renderer *r, int x, int y, Uint8 red, Uint8 green, Uint8 blue) {
    SDL_Rect rect = {
        x * TILE_SIZE + MARGIN_SIZE,
        y * TILE_SIZE + MARGIN_SIZE,
        TILE_SIZE - MARGIN_SIZE*2,
        TILE_SIZE - MARGIN_SIZE*2
    };
    SDL_SetRenderDrawColor(r, red, green, blue, 255);
    SDL_RenderFillRect(r, &rect);
}

/* Couleurs (avant de metttre des assets) :
terrain : Vert=Plaine  SapinFoncé=Forêt  Beige=Montagne  Bleu=Eau  Sable=Désert  GrisClair=Toundra
entités : Blanc=Ville  Orange=Camp  Rouge=Barbare  Jaune=Colon  Bleu=Guerrier */

static void draw_entities(SDL_Renderer *r, const GameState *gs) {
    for (City *c = gs->cities; c; c = c->next)
        draw_entity(r, c->x, c->y, 255, 255, 255);

    for (Camp *c = gs->camps; c; c = c->next)
        if (c->alive)
            draw_entity(r, c->x, c->y, 220, 120, 30);

    for (Unit *u = gs->units; u; u = u->next) {
        if (u->owner == UNIT_OWNER_BARB)
            draw_entity(r, u->x, u->y, 200, 40, 40);
        else if (u->type == UNIT_TYPE_SETTLER)
            draw_entity(r, u->x, u->y, 240, 220, 50);
        else
            draw_entity(r, u->x, u->y, 50, 150, 240);
    }
}

void sdl_run(GameState *gs) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "SDL_Init: %s\n", SDL_GetError());
        return;
    }

    int screen_w = gs->map.width  * TILE_SIZE;
    int screen_h = gs->map.height * TILE_SIZE;

    SDL_Window *win = SDL_CreateWindow(
        "Early Civilization",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        screen_w, screen_h,
        SDL_WINDOW_SHOWN
    );
    if (!win) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "SDL_CreateWindow: %s\n", SDL_GetError());
        SDL_Quit();
        return;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(
        win, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "SDL_CreateRenderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        return;
    }

    int running = 1;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
                running = 0;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        draw_map(renderer, gs);
        draw_entities(renderer, gs);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
}
