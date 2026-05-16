#include "save.h"
#include "core.h"
#include "turn.h"
#include "cities.h"
#include "units.h"
#include "lifecycle.h"
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>

static const char *result_str(GameResult r) {
    switch (r) {
        case RESULT_WIN_SCI: return "VICTOIRE_SCI";
        case RESULT_WIN_TERR: return "VICTOIRE_TERR";
        case RESULT_LOSS_NOCITY: return "DEFAITE_NOCITY";
        case RESULT_LOSS_TURNS: return "DEFAITE_TURNS";
        case RESULT_LOSS_PROD: return "DEFAITE_PROD";
        default: return "INCONNU";
    }
}

void scores_append(GameState *gs) {
    FILE *f = fopen(SCORES_FILE, "a");
    if (!f) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char date[16];
    strftime(date, sizeof(date), "%Y-%m-%d", t);

    fprintf(f, "%s %s %d tour:%d seed:%s\n",
            date,
            result_str(gs->result),
            game_compute_score(gs),
            gs->turn,
            gs->seed.hex);
    fclose(f);
}

void game_save(GameState *gs) {
    mkdir(SAVES_DIR, 0755);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char path[128];
    strftime(path, sizeof(path), SAVES_DIR "/save_tour%%d_%Y%m%d_%H%M.txt", t);
    char final_path[128];
    snprintf(final_path, sizeof(final_path), path, gs->turn);

    FILE *f = fopen(final_path, "w");
    if (!f) return;

    fprintf(f, "turn %d\n", gs->turn);
    fprintf(f, "turn_max %d\n", gs->turn_max);
    fprintf(f, "gold %d\n", gs->gold);
    fprintf(f, "n_barb_camps_initial %d\n", gs->n_barbarian_camps_initial);
    fprintf(f, "n_camps_destroyed %d\n", gs->n_camps_destroyed);
    fprintf(f, "barb_unit_count %d\n", gs->barb_unit_count);
    fprintf(f, "territorial_turns %d\n", gs->territorial_consecutive_turns);
    fprintf(f, "zero_prod_turns %d\n", gs->zero_prod_turns);
    fprintf(f, "next_unit_id %d\n", gs->next_unit_id);
    fprintf(f, "next_city_id %d\n", gs->next_city_id);
    fprintf(f, "next_camp_id %d\n", gs->next_camp_id);
    fprintf(f, "seed_hex %s\n", gs->seed.hex);
    fprintf(f, "result %d\n", (int)gs->result);

    fprintf(f, "MAP %d %d\n", gs->map.width, gs->map.height);
    for (int y = 0; y < gs->map.height; y++) {
        for (int x = 0; x < gs->map.width; x++)
            fprintf(f, "%d %d %d ", (int)gs->map.tiles[y][x].terrain,
                    gs->map.tiles[y][x].city_id, gs->map.tiles[y][x].camp_id);
        fprintf(f, "\n");
    }

    fprintf(f, "TECH %d %d\n", gs->tech.researching, gs->tech.science_stored);
    for (int i = 0; i < TECH_COUNT; i++)
        fprintf(f, "%d ", gs->tech.acquired[i]);
    fprintf(f, "\n");

    fprintf(f, "CITIES %d\n", cities_count(gs));
    for (City *c = gs->cities; c; c = c->next)
        fprintf(f, "%d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                c->id, c->x, c->y, c->population,
                c->food_stock, c->prod_stock, c->hp, c->hp_max,
                c->buildings, c->founded_turn,
                c->project_type, c->project_id, c->took_damage_this_turn);

    fprintf(f, "UNITS %d\n", units_count_all(gs));
    for (Unit *u = gs->units; u; u = u->next)
        fprintf(f, "%d %d %d %d %d %d %d %d %d %d %d %d\n",
                u->id, (int)u->type, (int)u->owner,
                u->x, u->y, u->hp, u->hp_max,
                u->attack, u->defense, u->pm_max, u->pm_left, u->maint);

    fprintf(f, "CAMPS %d\n", camps_count(gs));
    for (Camp *c = gs->camps; c; c = c->next)
        fprintf(f, "%d %d %d %d\n", c->id, c->x, c->y, c->alive);

    fclose(f);
    printf("Sauvegarde: %s\n", final_path);
}
