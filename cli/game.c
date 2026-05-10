
#include "game.h"

static char terrain_char(TerrainType t) {
    switch (t) {
        case TERRAIN_PLAIN:    return 'P';
        case TERRAIN_FOREST:   return 'F';
        case TERRAIN_MOUNTAIN: return 'M';
        case TERRAIN_WATER:    return '~';
        case TERRAIN_DESERT:   return 'D';
        case TERRAIN_TUNDRA:   return 'T';
        default:               return '?';
    }
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

void free_game(GameState *gs) {
    free_units(gs->units);
    free_cities(gs->cities);
    free_camps(gs->camps);
    free_map(&gs->map);
    free(gs);
}

/* ═══════════════════════════════════════════════════════════
   SETTLER COLONISE
═══════════════════════════════════════════════════════════ */

int game_settler_colonise(GameState *gs, Unit *settler) {
    if (settler->type != UNIT_TYPE_SETTLER || settler->owner != UNIT_OWNER_PLAYER) {
        printf("Seul un Colon peut coloniser.\n");
        return 0;
    }
    int x = settler->x, y = settler->y;

    /* Tile must be passable land */
    TerrainType t = gs->map.tiles[y][x].terrain;
    if (t == TERRAIN_WATER) {
        printf("Impossible de fonder une ville sur l'eau.\n");
        return 0;
    }

    /* No existing city or camp on tile */
    if (gs->map.tiles[y][x].city_id != -1 || gs->map.tiles[y][x].camp_id != -1) {
        printf("Tuile deja occupee.\n");
        return 0;
    }

    /* Distance check: >= CITY_MIN_DISTANCE from every other city */
    for (City *c = gs->cities; c; c = c->next) {
        int dx = c->x - x; if (dx < 0) dx = -dx;
        int dy = c->y - y; if (dy < 0) dy = -dy;
        int d = dx > dy ? dx : dy;
        if (d < CITY_MIN_DISTANCE) {
            printf("Trop proche d'une ville existante (distance Chebyshev: %d < %d).\n",
                   d, CITY_MIN_DISTANCE);
            return 0;
        }
    }

    /* Destroy the settler */
    int sid = settler->id;
    unit_destroy(gs, sid);

    /* Create city */
    city_create(gs, x, y, gs->turn);
    printf("[Fondation] Nouvelle ville en (%d,%d)!\n", x, y);
    return 1;
}

/* ═══════════════════════════════════════════════════════════
   GREEDY PATHFINDER  (for 'goto' command)
   Moves unit step-by-step toward (tx,ty) using Chebyshev-greedy
   same as barbarians, stopping when PM is exhausted.
═══════════════════════════════════════════════════════════ */

static const int MOVE_COSTS[TERRAIN_COUNT] = {
    TERRAIN_PLAIN_COST, TERRAIN_FOREST_COST, TERRAIN_MOUNTAIN_COST,
    TERRAIN_WATER_COST, TERRAIN_DESERT_COST, TERRAIN_TUNDRA_COST
};

static int cheb(int ax, int ay, int bx, int by) {
    int dx = ax - bx; if (dx < 0) dx = -dx;
    int dy = ay - by; if (dy < 0) dy = -dy;
    return dx > dy ? dx : dy;
}

static void unit_goto(GameState *gs, Unit *u, int tx, int ty) {
    while (u->pm_left > 0) {
        if (u->x == tx && u->y == ty) break;
        int best_dx = 0, best_dy = 0;
        int best_d = cheb(u->x, u->y, tx, ty);
        int best_cost = 999;

        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (dx == 0 && dy == 0) continue;
                int nx = u->x + dx, ny = u->y + dy;
                if (nx < 0 || nx >= gs->map.width || ny < 0 || ny >= gs->map.height) continue;
                TerrainType t = gs->map.tiles[ny][nx].terrain;
                int cost = MOVE_COSTS[t];
                if (cost == 0 || cost > u->pm_left) continue;
                /* Don't walk onto friendly units */
                Unit *occ = unit_at(gs, nx, ny, UNIT_OWNER_PLAYER);
                if (occ && occ->id != u->id) continue;
                int d = cheb(nx, ny, tx, ty);
                if (d < best_d || (d == best_d && cost < best_cost)) {
                    best_d = d; best_cost = cost;
                    best_dx = dx; best_dy = dy;
                }
            }
        }
        if (best_dx == 0 && best_dy == 0) break;
        int nx = u->x + best_dx, ny = u->y + best_dy;
        if (!unit_move(gs, u, nx, ny)) break;
        if (!unit_by_id(gs, u->id)) break; /* destroyed in combat */
    }
}

/* ═══════════════════════════════════════════════════════════
   GOLD / BANKRUPTCY
═══════════════════════════════════════════════════════════ */

static void apply_gold_phase(GameState *gs) {
    /* Compute income from all cities */
    int gold_income = 0;
    for (City *c = gs->cities; c; c = c->next)
        gold_income += c->yield_gold;

    /* Maintenance: units */
    int unit_maint_total = 0;
    for (Unit *u = gs->units; u; u = u->next)
        if (u->owner == UNIT_OWNER_PLAYER) unit_maint_total += u->maint;

    /* Maintenance: buildings */
    int building_maint_total = cities_total_building_maint(gs);

    int expenses = unit_maint_total + building_maint_total;
    gs->gold += gold_income - expenses;

    if (gs->gold < 0) {
        printf("[Banqueroute] Or < 0 (%d). ", gs->gold);
        /* Find a random player unit to dissolve */
        Unit *victim = NULL;
        for (Unit *u = gs->units; u; u = u->next)
            if (u->owner == UNIT_OWNER_PLAYER) { victim = u; break; }

        if (victim) {
            printf("Unite dissolue: %s id=%d\n", unit_type_name(victim->type), victim->id);
            unit_destroy(gs, victim->id);
            gs->gold = 0;
        } else {
            printf("Aucune unite a dissoudre. Production de toutes les villes divisee par 2.\n");
            for (City *c = gs->cities; c; c = c->next)
                c->yield_prod /= 2;
        }
    }
}

/* ═══════════════════════════════════════════════════════════
   SCIENCE PHASE
═══════════════════════════════════════════════════════════ */

static void apply_science_phase(GameState *gs) {
    int total_sci = 0;
    for (City *c = gs->cities; c; c = c->next)
        total_sci += c->yield_science;

    if (gs->tech.researching != TECH_ID_NONE) {
        int completed = tech_add_science(&gs->tech, total_sci);
        if (completed != TECH_ID_NONE) {
            tech_apply_effects(gs);
        }
    }
}

/* ═══════════════════════════════════════════════════════════
   VICTORY / DEFEAT CHECK
═══════════════════════════════════════════════════════════ */

GameResult game_check_result(GameState *gs) {
    /* Scientific victory */
    if (tech_all_acquired(&gs->tech))
        return RESULT_WIN_SCI;

    /* Territorial victory: >= 10 cities for 5 consecutive turns */
    if (cities_count(gs) >= VICTORY_TERRITORIAL_CITIES) {
        gs->territorial_consecutive_turns++;
        if (gs->territorial_consecutive_turns >= VICTORY_TERRITORIAL_TURNS)
            return RESULT_WIN_TERR;
    } else {
        gs->territorial_consecutive_turns = 0;
    }

    /* Defeat: no cities */
    if (cities_count(gs) == 0)
        return RESULT_LOSS_NOCITY;

    /* Defeat: turn limit */
    if (gs->turn > gs->turn_max)
        return RESULT_LOSS_TURNS;

    /* Defeat: total production = 0 for N consecutive turns */
    int total_prod = 0;
    for (City *c = gs->cities; c; c = c->next) total_prod += c->yield_prod;
    if (total_prod == 0) {
        gs->zero_prod_turns++;
        if (gs->zero_prod_turns >= GOLD_BANKRUPT_PROD_HALVE_TURNS)
            return RESULT_LOSS_PROD;
    } else {
        gs->zero_prod_turns = 0;
    }

    return RESULT_ONGOING;
}

int game_compute_score(GameState *gs) {
    int n_tech = 0;
    for (int i = 0; i < TECH_COUNT; i++)
        if (gs->tech.acquired[i]) n_tech++;

    int pop   = city_total_pop(gs);
    int n_cit = cities_count(gs);
    int turns_left = gs->turn_max - gs->turn;
    if (turns_left < 0) turns_left = 0;

    int score = n_tech * SCORE_TECH_WEIGHT
              + pop    * SCORE_POP_WEIGHT
              + n_cit  * SCORE_CITY_WEIGHT;

    if (gs->result == RESULT_WIN_SCI || gs->result == RESULT_WIN_TERR)
        score += turns_left * SCORE_TURN_BONUS;

    gs->score = score;
    return score;
}

/* ═══════════════════════════════════════════════════════════
   PHASE FUNCTIONS
═══════════════════════════════════════════════════════════ */

void game_phase_action(GameState *gs) {
    /* Reset PM for player units */
    units_reset_pm(gs, UNIT_OWNER_PLAYER);

    cli_print_map(gs);
    cli_print_status(gs);
    cli_print_help();

    char line[256];
    while (1) {
        printf("> ");
        fflush(stdout);
        if (!fgets(line, sizeof line, stdin)) break;

        /* strip trailing newline */
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') line[len-1] = '\0';

        /* Tokenise */
        char *cmd = strtok(line, " ");
        if (!cmd) continue;

        if (strcmp(cmd, "end") == 0) break;

        else if (strcmp(cmd, "quit") == 0) {
            printf("Au revoir.\n");
            game_compute_score(gs);
            cli_print_result(gs);
            free_game(gs);
            exit(0);
        }

        else if (strcmp(cmd, "m") == 0) {
            cli_print_map(gs);
        }
        else if (strcmp(cmd, "s") == 0) {
            cli_print_status(gs);
        }
        else if (strcmp(cmd, "u") == 0) {
            cli_print_units(gs);
        }
        else if (strcmp(cmd, "c") == 0) {
            cli_print_cities(gs);
        }
        else if (strcmp(cmd, "t") == 0) {
            cli_print_tech(gs);
        }
        else if (strcmp(cmd, "help") == 0) {
            cli_print_help();
        }

        else if (strcmp(cmd, "move") == 0) {
            char *s_id = strtok(NULL, " ");
            char *s_dx = strtok(NULL, " ");
            char *s_dy = strtok(NULL, " ");
            if (!s_id || !s_dx || !s_dy) { printf("Usage: move <id> <dx> <dy>\n"); continue; }
            int uid = atoi(s_id);
            int dx  = atoi(s_dx);
            int dy  = atoi(s_dy);
            Unit *u = unit_by_id(gs, uid);
            if (!u || u->owner != UNIT_OWNER_PLAYER) { printf("Unite invalide.\n"); continue; }
            int ok = unit_move(gs, u, u->x + dx, u->y + dy);
            if (!ok) printf("Deplacement impossible.\n");
        }

        else if (strcmp(cmd, "goto") == 0) {
            char *s_id = strtok(NULL, " ");
            char *s_x  = strtok(NULL, " ");
            char *s_y  = strtok(NULL, " ");
            if (!s_id || !s_x || !s_y) { printf("Usage: goto <id> <x> <y>\n"); continue; }
            int uid = atoi(s_id);
            int tx  = atoi(s_x);
            int ty  = atoi(s_y);
            Unit *u = unit_by_id(gs, uid);
            if (!u || u->owner != UNIT_OWNER_PLAYER) { printf("Unite invalide.\n"); continue; }
            unit_goto(gs, u, tx, ty);
        }

        else if (strcmp(cmd, "col") == 0) {
            char *s_id = strtok(NULL, " ");
            if (!s_id) { printf("Usage: col <id>\n"); continue; }
            int uid = atoi(s_id);
            Unit *u = unit_by_id(gs, uid);
            if (!u) { printf("Unite invalide.\n"); continue; }
            game_settler_colonise(gs, u);
        }

        else if (strcmp(cmd, "proj") == 0) {
            char *s_cid  = strtok(NULL, " ");
            char *s_type = strtok(NULL, " ");
            char *s_pid  = strtok(NULL, " ");
            if (!s_cid || !s_type || !s_pid) {
                printf("Usage: proj <city_id> <u|b> <id>\n");
                printf("  u = unite: 0=Colon 1=Guerrier\n");
                printf("  b = batiment: 1=Grenier 2=Atelier 4=Biblio 8=Marche 16=Caserne 32=Muraille\n");
                continue;
            }
            int cid = atoi(s_cid);
            int pid = atoi(s_pid);
            City *city = city_by_id(gs, cid);
            if (!city) { printf("Ville invalide.\n"); continue; }

            if (s_type[0] == 'u') {
                UnitType ut = (UnitType)pid;
                if (ut == UNIT_TYPE_WARRIOR && !gs->warrior_unlocked) {
                    printf("Artisanat requis pour produire un Guerrier.\n"); continue;
                }
                city_set_project(city, PROJECT_UNIT, pid);
                printf("[Ville %d] Projet: %s\n", cid, unit_type_name(ut));
            } else if (s_type[0] == 'b') {
                if (!city_can_build(gs, city, pid)) {
                    printf("Batiment non disponible (deja construit ou prérequis manquant).\n"); continue;
                }
                city_set_project(city, PROJECT_BUILDING, pid);
                printf("[Ville %d] Projet: %s\n", cid, building_name(pid));
            } else {
                printf("Type invalide (u ou b).\n");
            }
        }

        else if (strcmp(cmd, "research") == 0) {
            char *s_id = strtok(NULL, " ");
            if (!s_id) { cli_print_tech(gs); continue; }
            int tid = atoi(s_id);
            if (!tech_available(&gs->tech, tid)) {
                printf("Technologie non disponible (deja acquise ou prerequis manquant).\n"); continue;
            }
            tech_set_research(&gs->tech, tid);
            printf("Recherche lancee: %s\n", tech_name(tid));
        }

        else {
            printf("Commande inconnue. Tapez 'help'.\n");
        }
    }
}

void game_phase_production(GameState *gs) {
    /* Yields were computed at start of action phase and again here for accuracy */
    cities_compute_yields(gs);
    cities_produce(gs);
    apply_gold_phase(gs);
    apply_science_phase(gs);
}

void game_phase_growth(GameState *gs) {
    cities_grow(gs);
}

void game_phase_barbarians(GameState *gs) {
    barbarians_turn(gs);
}

void game_phase_end(GameState *gs) {
    cities_heal(gs);
    cities_reset_damage_flag(gs);
    gs->result = game_check_result(gs);
}

/* ═══════════════════════════════════════════════════════════
   MAIN LOOP
═══════════════════════════════════════════════════════════ */

void game_run(GameState *gs) {
    /* Compute initial yields for display */
    cities_compute_yields(gs);

    while (gs->result == RESULT_ONGOING) {
        game_phase_action(gs);
        if (gs->result != RESULT_ONGOING) break; /* quit called inside action */

        game_phase_production(gs);
        game_phase_growth(gs);
        game_phase_barbarians(gs);
        game_phase_end(gs);

        if (gs->result == RESULT_ONGOING) {
            gs->turn++;
            /* Recompute yields for next turn display */
            cities_compute_yields(gs);
        }
    }

    game_compute_score(gs);
    cli_print_result(gs);
}
