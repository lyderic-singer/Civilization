#include "save.h"
#include "core.h"
#include "turn.h"
#include "cities.h"
#include "units.h"
#include "lifecycle.h"
#include "utils.h"
#include "tech.h"
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

GameState *game_load(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return NULL;

    GameState *gs = calloc(1, sizeof(GameState));
    if (!gs) { fclose(f); return NULL; }

    char seed_hex[SEED_HEX_LEN + 1];
    fscanf(f, "turn %d\n", &gs->turn);
    fscanf(f, "turn_max %d\n", &gs->turn_max);
    fscanf(f, "gold %d\n", &gs->gold);
    fscanf(f, "n_barb_camps_initial %d\n", &gs->n_barbarian_camps_initial);
    fscanf(f, "n_camps_destroyed %d\n", &gs->n_camps_destroyed);
    fscanf(f, "barb_unit_count %d\n", &gs->barb_unit_count);
    fscanf(f, "territorial_turns %d\n", &gs->territorial_consecutive_turns);
    fscanf(f, "zero_prod_turns %d\n", &gs->zero_prod_turns);
    fscanf(f, "next_unit_id %d\n", &gs->next_unit_id);
    fscanf(f, "next_city_id %d\n", &gs->next_city_id);
    fscanf(f, "next_camp_id %d\n", &gs->next_camp_id);
    fscanf(f, "seed_hex %s\n", seed_hex);
    fscanf(f, "result %d\n", (int *)&gs->result);
    gs->seed = gen_seed(seed_hex);

    int W, H;
    fscanf(f, "MAP %d %d\n", &W, &H);
    gs->map.width = W;
    gs->map.height = H;
    gs->map.tiles = malloc(H * sizeof(Tile *));
    for (int y = 0; y < H; y++) {
        gs->map.tiles[y] = malloc(W * sizeof(Tile));
        for (int x = 0; x < W; x++) {
            int terrain, city_id, camp_id;
            fscanf(f, "%d %d %d ", &terrain, &city_id, &camp_id);
            gs->map.tiles[y][x].terrain = (TerrainType)terrain;
            gs->map.tiles[y][x].city_id = city_id;
            gs->map.tiles[y][x].camp_id = camp_id;
        }
        fscanf(f, "\n");
    }

    int researching, science_stored;
    fscanf(f, "TECH %d %d\n", &researching, &science_stored);
    gs->tech.researching = researching;
    gs->tech.science_stored = science_stored;
    for (int i = 0; i < TECH_COUNT; i++)
        fscanf(f, "%d ", &gs->tech.acquired[i]);
    fscanf(f, "\n");
    tech_apply_effects(gs);

    int nc;
    fscanf(f, "CITIES %d\n", &nc);
    for (int i = 0; i < nc; i++) {
        City *c = calloc(1, sizeof(City));
        fscanf(f, "%d %d %d %d %d %d %d %d %d %d %d %d %d\n",
               &c->id, &c->x, &c->y, &c->population,
               &c->food_stock, &c->prod_stock, &c->hp, &c->hp_max,
               &c->buildings, &c->founded_turn,
               &c->project_type, &c->project_id, &c->took_damage_this_turn);
        c->next = gs->cities;
        gs->cities = c;
    }

    int nu;
    fscanf(f, "UNITS %d\n", &nu);
    for (int i = 0; i < nu; i++) {
        Unit *u = calloc(1, sizeof(Unit));
        int type, owner;
        fscanf(f, "%d %d %d %d %d %d %d %d %d %d %d %d\n",
               &u->id, &type, &owner,
               &u->x, &u->y, &u->hp, &u->hp_max,
               &u->attack, &u->defense, &u->pm_max, &u->pm_left, &u->maint);
        u->type = (UnitType)type;
        u->owner = (UnitOwner)owner;
        u->next = gs->units;
        gs->units = u;
    }

    int ncamp;
    fscanf(f, "CAMPS %d\n", &ncamp);
    for (int i = 0; i < ncamp; i++) {
        Camp *c = calloc(1, sizeof(Camp));
        fscanf(f, "%d %d %d %d\n", &c->id, &c->x, &c->y, &c->alive);
        c->next = gs->camps;
        gs->camps = c;
    }

    fclose(f);
    cities_compute_yields(gs);
    units_reset_pm(gs, UNIT_OWNER_PLAYER);
    gs->result = RESULT_ONGOING;
    return gs;
}
