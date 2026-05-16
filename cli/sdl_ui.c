#include "sdl_ui.h"
#include "../core/turn.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdint.h>

typedef struct { uint8_t r, g, b; } Color;
typedef struct { int x, y; } Camera;

static const Color TERRAIN_COLORS[TERRAIN_COUNT] = {
    [TERRAIN_PLAIN] = { 144, 200, 100 },
    [TERRAIN_FOREST] = {  34, 100,  34 },
    [TERRAIN_MOUNTAIN] = { 150, 130, 100 },
    [TERRAIN_WATER] = {  64, 120, 200 },
    [TERRAIN_DESERT] = { 230, 200, 100 },
    [TERRAIN_TUNDRA] = { 200, 220, 230 },
};

static const SDL_Color COL_WHITE = { 255, 255, 255, 255 };
static const SDL_Color COL_YELLOW = { 255, 210,  50, 255 };
static const SDL_Color COL_CYAN = { 100, 200, 255, 255 };
static const SDL_Color COL_GRAY = { 160, 160, 160, 255 };
static const SDL_Color COL_CITY = { 255, 255, 255, 255 };
static const SDL_Color COL_CAMP = { 220, 120,  30, 255 };
static const SDL_Color COL_BARB = { 200,  40,  40, 255 };
static const SDL_Color COL_SETTLER = { 240, 220,  50, 255 };
static const SDL_Color COL_WARRIOR = {  50, 150, 240, 255 };
static const SDL_Color COL_SELECT = { 255, 255,   0, 255 };

static void camera_clamp(Camera *cam, GameState *gs) {
    int tiles_x = SCREEN_W / TILE_SIZE;
    int tiles_y = SCREEN_H / TILE_SIZE;
    if (cam->x < 0) cam->x = 0;
    if (cam->y < 0) cam->y = 0;
    int max_x = gs->map.width  - tiles_x;
    int max_y = gs->map.height - tiles_y;
    if (max_x <= 0) cam->x = 0; else if (cam->x > max_x) cam->x = max_x;
    if (max_y <= 0) cam->y = 0; else if (cam->y > max_y) cam->y = max_y;
}

static void draw_tile(SDL_Renderer *r, int x, int y, TerrainType t) {
    Color c = TERRAIN_COLORS[t];
    SDL_Rect rect = { x, y, TILE_SIZE, TILE_SIZE };
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, 255);
    SDL_RenderFillRect(r, &rect);
    SDL_SetRenderDrawColor(r, c.r / 2, c.g / 2, c.b / 2, 255);
    SDL_RenderDrawRect(r, &rect);
}

static void draw_map(SDL_Renderer *r, GameState *gs, Camera cam) {
    int tiles_x = SCREEN_W / TILE_SIZE;
    int tiles_y = SCREEN_H / TILE_SIZE;
    for (int sy = 0; sy <= tiles_y; sy++) {
        for (int sx = 0; sx <= tiles_x; sx++) {
            int mx = sx + cam.x;
            int my = sy + cam.y;
            if (mx < 0 || mx >= gs->map.width || my < 0 || my >= gs->map.height) continue;
            draw_tile(r, sx * TILE_SIZE, sy * TILE_SIZE,
                      gs->map.tiles[my][mx].terrain);
        }
    }
}

static void draw_entity(SDL_Renderer *r, int tx, int ty, SDL_Color color, Camera cam) {
    int sx = (tx - cam.x) * TILE_SIZE;
    int sy = (ty - cam.y) * TILE_SIZE;
    if (sx < 0 || sx >= SCREEN_W || sy < 0 || sy >= SCREEN_H) return;
    SDL_Rect rect = {
        sx + MARGIN_SIZE,
        sy + MARGIN_SIZE,
        TILE_SIZE - MARGIN_SIZE * 2,
        TILE_SIZE - MARGIN_SIZE * 2
    };
    SDL_SetRenderDrawColor(r, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(r, &rect);
}

static void draw_selection(SDL_Renderer *r, int tx, int ty, Camera cam) {
    int sx = (tx - cam.x) * TILE_SIZE;
    int sy = (ty - cam.y) * TILE_SIZE;
    if (sx < 0 || sx >= SCREEN_W || sy < 0 || sy >= SCREEN_H) return;
    SDL_Rect rect = { sx, sy, TILE_SIZE, TILE_SIZE };
    SDL_SetRenderDrawColor(r, COL_SELECT.r, COL_SELECT.g, COL_SELECT.b, COL_SELECT.a);
    SDL_RenderDrawRect(r, &rect);
    SDL_Rect inner = { rect.x + 1, rect.y + 1, rect.w - 2, rect.h - 2 };
    SDL_RenderDrawRect(r, &inner);
}

static void handle_click(GameState *gs, int px, int py, int *selected_id, Camera cam) {
    int tx = px / TILE_SIZE + cam.x;
    int ty = py / TILE_SIZE + cam.y;

    if (tx < 0 || tx >= gs->map.width || ty < 0 || ty >= gs->map.height) return;
    if (py >= SCREEN_H) return;

    Unit *pu = unit_at(gs, tx, ty, UNIT_OWNER_PLAYER);

    if (*selected_id == -1) {
        if (pu) *selected_id = pu->id;
    } else {
        Unit *u = unit_by_id(gs, *selected_id);
        if (u) unit_goto(gs, u, tx, ty);
        *selected_id = -1;
    }
}

static void draw_text(SDL_Renderer *r, TTF_Font *font,
                      const char *text, int x, int y, SDL_Color color) {
    SDL_Surface *surf = TTF_RenderText_Blended(font, text, color);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(r, surf);
    SDL_FreeSurface(surf);
    if (!tex) return;
    int w, h;
    SDL_QueryTexture(tex, NULL, NULL, &w, &h);
    SDL_Rect dst = { x, y, w, h };
    SDL_RenderCopy(r, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
}

static int text_width(TTF_Font *font, const char *text) {
    int w = 0, h = 0;
    TTF_SizeText(font, text, &w, &h);
    return w;
}

static void draw_stat(SDL_Renderer *r, TTF_Font *font,
                      int *x, int y, const char *text, SDL_Color color) {
    draw_text(r, font, text, *x, y, color);
    *x += text_width(font, text) + GAP;
}

static void draw_legend_item(SDL_Renderer *r, TTF_Font *font,
                             int *x, int y, SDL_Color color, const char *label) {
    SDL_Rect sq = { *x, y + 3, FONT_SIZE - 6, FONT_SIZE - 6 };
    SDL_SetRenderDrawColor(r, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(r, &sq);
    draw_text(r, font, label, *x + FONT_SIZE - 2, y, COL_WHITE);
    *x += FONT_SIZE - 2 + text_width(font, label) + GAP;
}

static void draw_panel(SDL_Renderer *r, TTF_Font *font,
                       GameState *gs, int selected_id) {
    SDL_SetRenderDrawColor(r, 20, 20, 30, 255);
    SDL_Rect bg = { 0, SCREEN_H, SCREEN_W, PANEL_H };
    SDL_RenderFillRect(r, &bg);

    SDL_SetRenderDrawColor(r, 80, 80, 100, 255);
    SDL_RenderDrawLine(r, 0, SCREEN_H, SCREEN_W, SCREEN_H);

    char buf[128];
    int y1 = SCREEN_H + PANEL_PAD;
    int y2 = y1 + LINE_H;
    int y3 = y2 + LINE_H;
    int x = PANEL_PAD;

    snprintf(buf, sizeof(buf), "Tour %d/%d", gs->turn, gs->turn_max);
    draw_stat(r, font, &x, y1, buf, COL_WHITE);

    snprintf(buf, sizeof(buf), "Or: %d", gs->gold);
    draw_stat(r, font, &x, y1, buf, COL_YELLOW);

    if (gs->tech.researching != TECH_ID_NONE) {
        const TechDef *td = tech_def(gs->tech.researching);
        snprintf(buf, sizeof(buf), "Recherche: %s [%d/%d]",
                 td->name, gs->tech.science_stored, td->cost);
        draw_stat(r, font, &x, y1, buf, COL_CYAN);
    } else {
        draw_stat(r, font, &x, y1, "Recherche: aucune (T pour choisir)", COL_GRAY);
    }

    if (selected_id != -1) {
        Unit *u = unit_by_id(gs, selected_id);
        if (u) {
            snprintf(buf, sizeof(buf), "| %s  PV:%d/%d  PM:%d/%d",
                     unit_type_name(u->type), u->hp, u->hp_max, u->pm_left, u->pm_max);
            draw_text(r, font, buf, x, y1, COL_WHITE);
        }
    }

    draw_text(r, font, "[Entree=fin de tour]  [C=coloniser]  [T=tech]  [Fleches=scroll]  [Echap=quitter]",
              PANEL_PAD, y2, COL_GRAY);

    x = PANEL_PAD;
    draw_legend_item(r, font, &x, y3, COL_CITY,"Ville");
    draw_legend_item(r, font, &x, y3, COL_CAMP,"Camp");
    draw_legend_item(r, font, &x, y3, COL_BARB,"Barbare");
    draw_legend_item(r, font, &x, y3, COL_SETTLER, "Colon");
    draw_legend_item(r, font, &x, y3, COL_WARRIOR, "Guerrier");
}

static void draw_entities(SDL_Renderer *r, GameState *gs, Camera cam) {
    for (City *c = gs->cities; c; c = c->next)
        draw_entity(r, c->x, c->y, COL_CITY, cam);

    for (Camp *c = gs->camps; c; c = c->next)
        if (c->alive)
            draw_entity(r, c->x, c->y, COL_CAMP, cam);

    for (Unit *u = gs->units; u; u = u->next) {
        if (u->owner == UNIT_OWNER_BARB)
            draw_entity(r, u->x, u->y, COL_BARB, cam);
        else if (u->type == UNIT_TYPE_SETTLER)
            draw_entity(r, u->x, u->y, COL_SETTLER, cam);
        else
            draw_entity(r, u->x, u->y, COL_WARRIOR, cam);
    }
}

void sdl_run(GameState *gs) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "SDL_Init: %s\n", SDL_GetError());
        return;
    }
    if (TTF_Init() != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "TTF_Init: %s\n", TTF_GetError());
        SDL_Quit();
        return;
    }

    SDL_Window *win = SDL_CreateWindow(
        "Early Civilization",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_W, SCREEN_H + PANEL_H,
        SDL_WINDOW_SHOWN
    );
    if (!win) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "SDL_CreateWindow: %s\n", SDL_GetError());
        TTF_Quit();
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
        TTF_Quit();
        SDL_Quit();
        return;
    }



    TTF_Font *font = TTF_OpenFont(FONT_PATH, FONT_SIZE);
    if (!font) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "TTF_OpenFont: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(win);
        TTF_Quit();
        SDL_Quit();
        return;
    }

    cities_compute_yields(gs);
    units_reset_pm(gs, UNIT_OWNER_PLAYER);

    Camera cam = { 0, 0 };

    int running     = 1;
    int selected_id = -1;

    while (running && gs->result == RESULT_ONGOING) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;

            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE: running = 0; break;
                    case SDLK_LEFT:   cam.x--; camera_clamp(&cam, gs); break;
                    case SDLK_RIGHT:  cam.x++; camera_clamp(&cam, gs); break;
                    case SDLK_UP:     cam.y--; camera_clamp(&cam, gs); break;
                    case SDLK_DOWN:   cam.y++; camera_clamp(&cam, gs); break;
                    case SDLK_RETURN:
                        selected_id = -1;
                        game_phase_production(gs);
                        cities_grow(gs);
                        barbarians_turn(gs);
                        game_phase_end(gs);
                        if (gs->result == RESULT_ONGOING) {
                            gs->turn++;
                            cities_compute_yields(gs);
                            units_reset_pm(gs, UNIT_OWNER_PLAYER);
                        }
                        break;
                }
            }

            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT)
                handle_click(gs, e.button.x, e.button.y, &selected_id, cam);
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        draw_map(renderer, gs, cam);
        draw_entities(renderer, gs, cam);
        draw_panel(renderer, font, gs, selected_id);
        if (selected_id != -1) {
            Unit *u = unit_by_id(gs, selected_id);
            if (u) draw_selection(renderer, u->x, u->y, cam);
            else   selected_id = -1;
        }
        SDL_RenderPresent(renderer);
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    TTF_Quit();
    SDL_Quit();
}
