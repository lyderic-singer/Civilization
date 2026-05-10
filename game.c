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

GameState *gen_game(int width, int height, int turns, int n_barb_camps, Seed seed) {
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
    tech_apply_effects(gs); /* zero everything out cleanly */

    gen_map(gs);

    printf("\n=== Early Civilization ===\n");
    printf("Seed: %s\n", gs->seed.hex);
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

/* ═══════════════════════════════════════════════════════════
   CLI DISPLAY
═══════════════════════════════════════════════════════════ */

void cli_print_map(GameState *gs) {
    int W = gs->map.width;
    int H = gs->map.height;

    /* Top ruler */
    printf("   ");
    for (int x = 0; x < W; x++) {
        if (x % 10 == 0) printf("|");
        else              printf("%d", x % 10);
    }
    printf("\n");

    for (int y = 0; y < H; y++) {
        printf("%2d ", y);
        for (int x = 0; x < W; x++) {
            /* Overlay priority: player unit > barb unit > city > camp > terrain */
            Unit *pu = unit_at(gs, x, y, UNIT_OWNER_PLAYER);
            Unit *bu = unit_at(gs, x, y, UNIT_OWNER_BARB);
            int cid  = gs->map.tiles[y][x].city_id;
            int pid  = gs->map.tiles[y][x].camp_id;

            char ch;
            if (pu) {
                ch = (pu->type == UNIT_TYPE_SETTLER) ? 'S' : 'W';
            } else if (cid != -1) {
                ch = 'C';
            } else if (bu) {
                ch = 'b';
            } else if (pid != -1) {
                /* check camp alive */
                int alive = 0;
                for (Camp *c = gs->camps; c; c = c->next)
                    if (c->id == pid && c->alive) { alive = 1; break; }
                ch = alive ? 'X' : terrain_char(gs->map.tiles[y][x].terrain);
            } else {
                ch = terrain_char(gs->map.tiles[y][x].terrain);
            }
            putchar(ch);
        }
        printf("\n");
    }

    printf("\nLegende: .=Plaine T=Foret M=Montagne ~=Eau D=Desert _=Toundra\n");
    printf("         C=Ville S=Colon W=Guerrier b=Barbare X=Camp barbare\n\n");
}

void cli_print_status(GameState *gs) {
    printf("--- Tour %d/%d --- Or: %d --- Science en cours: %d/%d (%s) ---\n",
           gs->turn, gs->turn_max, gs->gold,
           gs->tech.science_stored,
           gs->tech.researching != TECH_ID_NONE ? tech_def(gs->tech.researching)->cost : 0,
           gs->tech.researching != TECH_ID_NONE ? tech_name(gs->tech.researching) : "aucune");
}

void cli_print_units(GameState *gs) {
    printf("=== Unites du joueur ===\n");
    int found = 0;
    for (Unit *u = gs->units; u; u = u->next) {
        if (u->owner != UNIT_OWNER_PLAYER) continue;
        printf("  [%d] %s @ (%d,%d) PV:%d/%d ATK:%d DEF:%d PM:%d/%d\n",
               u->id, unit_type_name(u->type), u->x, u->y,
               u->hp, u->hp_max, u->attack, u->defense, u->pm_left, u->pm_max);
        found = 1;
    }
    if (!found) printf("  (aucune unite)\n");
}

void cli_print_cities(GameState *gs) {
    printf("=== Villes ===\n");
    int found = 0;
    for (City *c = gs->cities; c; c = c->next) {
        printf("  [%d] @ (%d,%d) Pop:%d PV:%d/%d | Nourriture:%d Prod:%d Or:%d Sci:%d\n",
               c->id, c->x, c->y, c->population, c->hp, c->hp_max,
               c->yield_food, c->yield_prod, c->yield_gold, c->yield_science);
        printf("       Batiments: ");
        if (c->buildings == BUILDING_NONE) printf("aucun");
        if (c->buildings & BUILDING_GRANARY)  printf("Grenier ");
        if (c->buildings & BUILDING_WORKSHOP) printf("Atelier ");
        if (c->buildings & BUILDING_LIBRARY)  printf("Bibliotheque ");
        if (c->buildings & BUILDING_MARKET)   printf("Marche ");
        if (c->buildings & BUILDING_BARRACKS) printf("Caserne ");
        if (c->buildings & BUILDING_WALL)     printf("Muraille ");
        printf("\n");
        if (c->project_type != PROJECT_NONE) {
            const char *proj_name = (c->project_type == PROJECT_BUILDING)
                ? building_name(c->project_id)
                : unit_type_name((UnitType)c->project_id);
            int proj_cost = city_project_cost(gs, c);
            printf("       Projet: %s [%d/%d]\n", proj_name, c->prod_stock, proj_cost);
        }
        found = 1;
    }
    if (!found) printf("  (aucune ville)\n");
}

void cli_print_tech(GameState *gs) {
    printf("=== Technologies ===\n");
    for (int i = 0; i < TECH_COUNT; i++) {
        const TechDef *d = tech_def(i);
        const char *status;
        if (gs->tech.acquired[i])       status = "[OK]";
        else if (gs->tech.researching == i) {
            static char buf[32];
            snprintf(buf, sizeof buf, "[%d/%d]", gs->tech.science_stored, d->cost);
            status = buf;
        }
        else if (tech_available(&gs->tech, i)) status = "[dispo]";
        else                                    status = "[verr.]";
        printf("  %d. %-15s %s  (cout:%d)\n", i, d->name, status, d->cost);
    }
}

void cli_print_help(void) {
    printf("\n=== Commandes ===\n");
    printf("  m           Afficher la carte\n");
    printf("  s           Afficher le statut\n");
    printf("  u           Lister les unites\n");
    printf("  c           Lister les villes\n");
    printf("  t           Arbre technologique\n");
    printf("  move <id> <dx> <dy>   Deplacer une unite\n");
    printf("  goto <id> <x> <y>    Deplacer une unite vers (x,y) (greedy)\n");
    printf("  col <id>             Coloniser (settler uniquement)\n");
    printf("  proj <city_id> <type> <id>   Definir projet (type: u=unite b=batiment)\n");
    printf("  research <tech_id>   Lancer recherche\n");
    printf("  end                  Passer au tour suivant\n");
    printf("  help                 Afficher cette aide\n");
    printf("  quit                 Quitter\n\n");
}

void cli_print_result(GameState *gs) {
    printf("\n");
    switch (gs->result) {
        case RESULT_WIN_SCI:
            printf("*** VICTOIRE SCIENTIFIQUE ***\n");
            printf("Vous avez acquis toutes les technologies!\n");
            break;
        case RESULT_WIN_TERR:
            printf("*** VICTOIRE TERRITORIALE ***\n");
            printf("Vous avez controle 10 villes pendant 5 tours consecutifs!\n");
            break;
        case RESULT_LOSS_NOCITY:
            printf("*** DEFAITE - Toutes vos villes ont ete detruites ***\n");
            break;
        case RESULT_LOSS_TURNS:
            printf("*** DEFAITE - Nombre de tours maximum atteint (%d) ***\n", gs->turn_max);
            break;
        case RESULT_LOSS_PROD:
            printf("*** DEFAITE - Production nulle pendant %d tours consecutifs ***\n",
                   GOLD_BANKRUPT_PROD_HALVE_TURNS);
            break;
        default: break;
    }
    printf("Score final: %d\n\n", gs->score);
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
            game_free(gs);
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

/* ═══════════════════════════════════════════════════════════
   ARGUMENT PARSING + ENTRY POINT
═══════════════════════════════════════════════════════════ */

static void print_usage(const char *prog) {
    printf("Usage: %s [options]\n", prog);
    printf("  --width  W  / -W W   Largeur de la carte (defaut: %d)\n", DEFAULT_WIDTH);
    printf("  --height H  / -H H   Hauteur de la carte (defaut: %d)\n", DEFAULT_HEIGHT);
    printf("  --seed N    / -s N   Graine (uint64) ou hex 40 chars\n");
    printf("  --turns N   / -t N   Nombre de tours max (defaut: %d)\n", DEFAULT_TURNS);
    printf("  --barbarians N / -b N  Camps barbares (defaut: %d)\n", DEFAULT_BARBARIANS);
    printf("  --mode MODE / -m MODE  Mode: cli (seul supporte)\n");
    printf("  --help\n");
}

int game_main(int argc, char **argv) {
    int    width   = DEFAULT_WIDTH;
    int    height  = DEFAULT_HEIGHT;
    int    turns   = DEFAULT_TURNS;
    int    n_barbs = DEFAULT_BARBARIANS;
    int    has_seed = 0;
    Seed   seed;

    for (int i = 1; i < argc; i++) {
        if ((strcmp(argv[i], "--width") == 0 || strcmp(argv[i], "-W") == 0) && i+1 < argc) {
            width = atoi(argv[++i]);
        } else if ((strcmp(argv[i], "--height") == 0 || strcmp(argv[i], "-H") == 0) && i+1 < argc) {
            height = atoi(argv[++i]);
        } else if ((strcmp(argv[i], "--turns") == 0 || strcmp(argv[i], "-t") == 0) && i+1 < argc) {
            turns = atoi(argv[++i]);
        } else if ((strcmp(argv[i], "--barbarians") == 0 || strcmp(argv[i], "-b") == 0) && i+1 < argc) {
            n_barbs = atoi(argv[++i]);
        } else if ((strcmp(argv[i], "--mode") == 0 || strcmp(argv[i], "-m") == 0) && i+1 < argc) {
            i++; /* only 'cli' supported */
        } else if ((strcmp(argv[i], "--seed") == 0 || strcmp(argv[i], "-s") == 0) && i+1 < argc) {
            seed = gen_seed(argv[++i]);
            has_seed = 1;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        } else {
            fprintf(stderr, "Parametre invalide: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    /* Validate */
    if (width < 10 || height < 10) {
        fprintf(stderr, "Carte trop petite (minimum 10x10).\n");
        return 1;
    }
    if (turns < 1) {
        fprintf(stderr, "Nombre de tours invalide.\n");
        return 1;
    }
    if (n_barbs < 0) n_barbs = 0;
    if (!has_seed) seed = gen_random_seed();
    GameState *gs = gen_game(width, height, turns, n_barbs, seed);
    game_run(gs);
    game_free(gs);
    return 0;
}

/* ═══════════════════════════════════════════════════════════
   main()
═══════════════════════════════════════════════════════════ */

int main(int argc, char **argv) {
    return game_main(argc, argv);
}