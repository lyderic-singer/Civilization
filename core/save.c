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
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

#define CHECK(n, expr) if ((expr) != (n)) goto fail

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

    const char *pname = gs->player_name[0] ? gs->player_name : "Joueur";
    fprintf(f, "%s %s %s %d tour:%d seed:%s\n",
            pname,
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

    fprintf(f, "version %d\n", SAVE_VERSION);
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
    fprintf(f, "player_name %s\n", gs->player_name[0] ? gs->player_name : "Joueur");

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

int saves_list(char paths[][256], int max) {
    int n = 0;
    DIR *d = opendir(SAVES_DIR);
    if (!d) return 0;
    struct dirent *e;
    while ((e = readdir(d)) && n < max) {
        int len = strlen(e->d_name);
        if (len > 4 && strcmp(e->d_name + len - 4, ".txt") == 0) {
            snprintf(paths[n], 256, "%s/%s", SAVES_DIR, e->d_name);
            n++;
        }
    }
    closedir(d);
    return n;
}

static int cmp_score(const void *a, const void *b) {
    return ((ScoreEntry *)b)->score - ((ScoreEntry *)a)->score;
}

int scores_read(ScoreEntry *entries, int max) {
    FILE *f = fopen(SCORES_FILE, "r");
    if (!f) return 0;
    int n = 0;
    char line[128];
    while (n < max && fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\n")] = '\0';
        if (sscanf(line, "%31s %31s %31s %d tour:%d",
                   entries[n].name, entries[n].date, entries[n].result,
                   &entries[n].score, &entries[n].tour) == 5)
            n++;
    }
    fclose(f);
    qsort(entries, n, sizeof(ScoreEntry), cmp_score);
    return n;
}

void format_save_name(const char *filename, char *out, int outlen) {
    int tour, year, month, day, hour, minute;
    if (sscanf(filename, "save_tour%d_%4d%2d%2d_%2d%2d.txt",
               &tour, &year, &month, &day, &hour, &minute) == 6) {
        snprintf(out, outlen, "Tour %d  |  %02d/%02d/%04d  %02dh%02d",
                 tour, day, month, year, hour, minute);
    } else {
        snprintf(out, outlen, "%s", filename);
    }
}

GameState *game_load(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return NULL;

    GameState *gs = calloc(1, sizeof(GameState));
    if (!gs) { fclose(f); return NULL; }

    int version = 0;
    if (fscanf(f, "version %d\n", &version) != 1 || version != SAVE_VERSION) goto fail;
    char seed_hex[SEED_HEX_LEN + 1];
    CHECK(1, fscanf(f, "turn %d\n", &gs->turn));
    CHECK(1, fscanf(f, "turn_max %d\n", &gs->turn_max));
    CHECK(1, fscanf(f, "gold %d\n", &gs->gold));
    CHECK(1, fscanf(f, "n_barb_camps_initial %d\n", &gs->n_barbarian_camps_initial));
    CHECK(1, fscanf(f, "n_camps_destroyed %d\n", &gs->n_camps_destroyed));
    CHECK(1, fscanf(f, "barb_unit_count %d\n", &gs->barb_unit_count));
    CHECK(1, fscanf(f, "territorial_turns %d\n", &gs->territorial_consecutive_turns));
    CHECK(1, fscanf(f, "zero_prod_turns %d\n", &gs->zero_prod_turns));
    CHECK(1, fscanf(f, "next_unit_id %d\n", &gs->next_unit_id));
    CHECK(1, fscanf(f, "next_city_id %d\n", &gs->next_city_id));
    CHECK(1, fscanf(f, "next_camp_id %d\n", &gs->next_camp_id));
    CHECK(1, fscanf(f, "seed_hex %16s\n", seed_hex));
    CHECK(1, fscanf(f, "result %d\n", (int *)&gs->result));
    fscanf(f, "player_name %31s\n", gs->player_name);
    gs->seed = gen_seed(seed_hex);

    int W, H;
    CHECK(2, fscanf(f, "MAP %d %d\n", &W, &H));
    gs->map.width = W;
    gs->map.height = H;
    gs->map.tiles = calloc(H, sizeof(Tile *));
    if (!gs->map.tiles) goto fail;
    for (int y = 0; y < H; y++) {
        gs->map.tiles[y] = malloc(W * sizeof(Tile));
        if (!gs->map.tiles[y]) goto fail;
        for (int x = 0; x < W; x++) {
            int terrain, city_id, camp_id;
            CHECK(3, fscanf(f, "%d %d %d ", &terrain, &city_id, &camp_id));
            gs->map.tiles[y][x].terrain = (TerrainType)terrain;
            gs->map.tiles[y][x].city_id = city_id;
            gs->map.tiles[y][x].camp_id = camp_id;
        }
        fscanf(f, "\n");
    }

    int researching, science_stored;
    CHECK(2, fscanf(f, "TECH %d %d\n", &researching, &science_stored));
    gs->tech.researching = researching;
    gs->tech.science_stored = science_stored;
    for (int i = 0; i < TECH_COUNT; i++)
        CHECK(1, fscanf(f, "%d ", &gs->tech.acquired[i]));
    fscanf(f, "\n");
    tech_apply_effects(gs);

    int nc;
    CHECK(1, fscanf(f, "CITIES %d\n", &nc));
    for (int i = 0; i < nc; i++) {
        City *c = calloc(1, sizeof(City));
        if (!c) goto fail;
        c->next = gs->cities;
        gs->cities = c;
        CHECK(13, fscanf(f, "%d %d %d %d %d %d %d %d %d %d %d %d %d\n",
               &c->id, &c->x, &c->y, &c->population,
               &c->food_stock, &c->prod_stock, &c->hp, &c->hp_max,
               &c->buildings, &c->founded_turn,
               &c->project_type, &c->project_id, &c->took_damage_this_turn));
    }

    int nu;
    CHECK(1, fscanf(f, "UNITS %d\n", &nu));
    for (int i = 0; i < nu; i++) {
        Unit *u = calloc(1, sizeof(Unit));
        if (!u) goto fail;
        u->next = gs->units;
        gs->units = u;
        int type, owner;
        CHECK(12, fscanf(f, "%d %d %d %d %d %d %d %d %d %d %d %d\n",
               &u->id, &type, &owner,
               &u->x, &u->y, &u->hp, &u->hp_max,
               &u->attack, &u->defense, &u->pm_max, &u->pm_left, &u->maint));
        u->type = (UnitType)type;
        u->owner = (UnitOwner)owner;
    }

    int ncamp;
    CHECK(1, fscanf(f, "CAMPS %d\n", &ncamp));
    for (int i = 0; i < ncamp; i++) {
        Camp *c = calloc(1, sizeof(Camp));
        if (!c) goto fail;
        c->next = gs->camps;
        gs->camps = c;
        CHECK(4, fscanf(f, "%d %d %d %d\n", &c->id, &c->x, &c->y, &c->alive));
    }

    fclose(f);
    cities_compute_yields(gs);
    units_reset_pm(gs, UNIT_OWNER_PLAYER);
    gs->result = RESULT_ONGOING;
    return gs;

fail:
    fclose(f);
    free_game(gs);
    return NULL;
}
