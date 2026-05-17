#include "cli.h"
#include "../core/save.h"
#include <unistd.h>

int cli_choose_start(char *out_path) {
    char saves[32][256];
    ScoreEntry scores[64];

    while (1) {
        printf("\n=== Early Civilization ===\n");
        printf("1. Nouvelle partie\n");
        printf("2. Charger une sauvegarde\n");
        printf("3. Scores\n");
        printf("4. Quitter\n");
        printf("> ");
        fflush(stdout);

        char line[16];
        if (!fgets(line, sizeof(line), stdin)) return -1;
        int choice = atoi(line);

        if (choice == 1) return 0;
        if (choice == 4) return -1;

        if (choice == 2) {
            int n = saves_list(saves, 32);
            if (n == 0) { printf("Aucune sauvegarde trouvee.\n"); continue; }
            printf("\n--- Sauvegardes ---\n");
            for (int i = 0; i < n; i++) {
                char display[128];
                format_save_name(saves[i] + strlen(SAVES_DIR) + 1, display, sizeof(display));
                printf("  %d. %s\n", i + 1, display);
            }
            printf("  0. Retour\n> ");
            fflush(stdout);
            if (!fgets(line, sizeof(line), stdin)) continue;
            int idx = atoi(line) - 1;
            if (idx < 0 || idx >= n) continue;
            snprintf(out_path, 256, "%s", saves[idx]);
            return 1;
        }

        if (choice == 3) {
            int n = scores_read(scores, 64);
            printf("\n--- Tableau des scores ---\n");
            if (n == 0) { printf("Aucune partie terminee.\n"); continue; }
            printf("  %-12s %-18s %-8s Tour\n", "Date", "Resultat", "Score");
            for (int i = 0; i < n; i++)
                printf("  %-12s %-18s %-8d %d\n",
                       scores[i].date, scores[i].result, scores[i].score, scores[i].tour);
        }
    }
}

void cmd_move(GameState *gs, char *args) {
    char *s_id = strtok(args, " "), *s_dx = strtok(NULL, " "), *s_dy = strtok(NULL, " ");
    if (!s_id || !s_dx || !s_dy) { printf("Usage: move <id> <dx> <dy>\n"); return; }
    Unit *u = unit_by_id(gs, atoi(s_id));
    if (!u || u->owner != UNIT_OWNER_PLAYER) { printf("Unite invalide.\n"); return; }
    if (!unit_move(gs, u, u->x + atoi(s_dx), u->y + atoi(s_dy)))
        printf("Deplacement impossible.\n");
}

void cmd_goto(GameState *gs, char *args) {
    char *s_id = strtok(args, " "), *s_x = strtok(NULL, " "), *s_y = strtok(NULL, " ");
    if (!s_id || !s_x || !s_y) { printf("Usage: goto <id> <x> <y>\n"); return; }
    Unit *u = unit_by_id(gs, atoi(s_id));
    if (!u || u->owner != UNIT_OWNER_PLAYER) { printf("Unite invalide.\n"); return; }
    unit_goto(gs, u, atoi(s_x), atoi(s_y));
}

void cmd_col(GameState *gs, char *args) {
    char *s_id = strtok(args, " ");
    if (!s_id) { printf("Usage: col <id>\n"); return; }
    Unit *u = unit_by_id(gs, atoi(s_id));
    if (!u) { printf("Unite invalide.\n"); return; }
    game_settler_colonise(gs, u);
}

void cmd_proj(GameState *gs, char *args) {
    char *s_cid = strtok(args, " "), *s_type = strtok(NULL, " "), *s_pid = strtok(NULL, " ");
    if (!s_cid || !s_type || !s_pid) {
        printf("Usage: proj <city_id> <u|b> <id>\n");
        printf("  u = unite: 0=Colon 1=Guerrier\n");
        printf("  b = batiment: 1=Grenier 2=Atelier 4=Biblio 8=Marche 16=Caserne 32=Muraille\n");
        return;
    }
    int cid = atoi(s_cid), pid = atoi(s_pid);
    City *city = city_by_id(gs, cid);
    if (!city) { printf("Ville invalide.\n"); return; }
    if (s_type[0] == 'u') {
        UnitType ut = (UnitType)pid;
        if (ut == UNIT_TYPE_WARRIOR && !gs->warrior_unlocked) {
            printf("Artisanat requis pour produire un Guerrier.\n"); return;
        }
        city_set_project(city, PROJECT_UNIT, pid);
        printf("[Ville %d] Projet: %s\n", cid, unit_type_name(ut));
    } else if (s_type[0] == 'b') {
        if (!city_can_build(gs, city, pid)) {
            printf("Batiment non disponible (deja construit ou prérequis manquant).\n"); return;
        }
        city_set_project(city, PROJECT_BUILDING, pid);
        printf("[Ville %d] Projet: %s\n", cid, building_name(pid));
    } else {
        printf("Type invalide (u ou b).\n");
    }
}

void cmd_research(GameState *gs, char *args) {
    char *s_id = strtok(args, " ");
    if (!s_id) { print_tech(gs); return; }
    int tid = atoi(s_id);
    if (!tech_available(&gs->tech, tid)) {
        printf("Technologie non disponible (deja acquise ou prerequis manquant).\n"); return;
    }
    tech_set_research(&gs->tech, tid);
    printf("Recherche lancee: %s\n", tech_name(tid));
}

int cmd_dispatch(GameState *gs, char *cmd, char *args) {
    if (strcmp(cmd, "end") == 0) return 0;
    if (strcmp(cmd, "m") == 0) { print_map(gs); return 1; }
    if (strcmp(cmd, "s") == 0) { print_status(gs); return 1; }
    if (strcmp(cmd, "u") == 0) { print_units(gs); return 1; }
    if (strcmp(cmd, "c") == 0) { print_cities(gs); return 1; }
    if (strcmp(cmd, "t") == 0) { print_tech(gs); return 1; }
    if (strcmp(cmd, "help") == 0) { print_help(); return 1; }
    if (strcmp(cmd, "move") == 0) { cmd_move(gs, args); return 1; }
    if (strcmp(cmd, "goto") == 0) { cmd_goto(gs, args); return 1; }
    if (strcmp(cmd, "col") == 0) { cmd_col(gs, args); return 1; }
    if (strcmp(cmd, "proj") == 0) { cmd_proj(gs, args); return 1; }
    if (strcmp(cmd, "research") == 0) { cmd_research(gs, args); return 1; }
    if (strcmp(cmd, "quit") == 0) {
        printf("Au revoir.\n");
        gs->result = RESULT_LOSS_TURNS;
        return 0;
    }
    printf("Commande inconnue. Tapez 'help'.\n");
    return 1;
}

void game_phase_action(GameState *gs) {
    units_reset_pm(gs, UNIT_OWNER_PLAYER);
    print_map(gs); print_status(gs); print_help();
    char line[256];
    while (1) {
        printf("> "); fflush(stdout);
        if (!fgets(line, sizeof line, stdin)) break;
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') line[len-1] = '\0';
        char *cmd = strtok(line, " ");
        if (!cmd) continue;
        char *args = strtok(NULL, "");
        if (!cmd_dispatch(gs, cmd, args)) break;
    }
}

void game_run(GameState *gs) {
    cities_compute_yields(gs);
    while (gs->result == RESULT_ONGOING) {
        game_phase_action(gs);
        if (gs->result != RESULT_ONGOING) break;
        game_phase_production(gs);
        cities_grow(gs);
        barbarians_turn(gs);
        game_phase_end(gs);
        if (gs->result == RESULT_ONGOING) {
            gs->turn++;
            cities_compute_yields(gs);
    }}
    game_compute_score(gs);
    scores_append(gs);
    print_result(gs);
}

static const int TERRAIN_BG[TERRAIN_COUNT][3] = {
    [TERRAIN_PLAIN]    = { 144, 200, 100 },
    [TERRAIN_FOREST]   = {  34, 100,  34 },
    [TERRAIN_MOUNTAIN] = { 150, 130, 100 },
    [TERRAIN_WATER]    = {  64, 120, 200 },
    [TERRAIN_DESERT]   = { 230, 200, 100 },
    [TERRAIN_TUNDRA]   = { 200, 220, 230 },
};

static int has_truecolor(void) {
    if (!isatty(STDOUT_FILENO)) return 0;
    char *ct = getenv("COLORTERM");
    return ct && (strcmp(ct, "truecolor") == 0 || strcmp(ct, "24bit") == 0);
}

char terrain_char(TerrainType t) {
    switch (t) {
        case TERRAIN_PLAIN: return 'P';
        case TERRAIN_FOREST: return 'F';
        case TERRAIN_MOUNTAIN: return 'M';
        case TERRAIN_WATER: return '~';
        case TERRAIN_DESERT: return 'D';
        case TERRAIN_TUNDRA: return 'T';
        default: return '?';
    }
}

void print_map(GameState *gs) {
    int W = gs->map.width;
    int H = gs->map.height;
    int color = has_truecolor();

    printf("   ");
    for (int x = 0; x < W; x++) {
        if (x % 10 == 0) printf("|");
        else printf("%d", x % 10);
    }
    printf("\n");
    for (int y = 0; y < H; y++) {
        printf("%2d ", y);
        for (int x = 0; x < W; x++) {
            Unit *pu = unit_at(gs, x, y, UNIT_OWNER_PLAYER);
            Unit *bu = unit_at(gs, x, y, UNIT_OWNER_BARB);
            int cid = gs->map.tiles[y][x].city_id;
            int pid = gs->map.tiles[y][x].camp_id;
            int alive_camp = 0;
            if (pid != -1)
                for (Camp *c = gs->camps; c; c = c->next)
                    if (c->id == pid && c->alive) { alive_camp = 1; break; }

            if (color) {
                TerrainType t = gs->map.tiles[y][x].terrain;
                printf("\033[48;2;%d;%d;%dm", TERRAIN_BG[t][0], TERRAIN_BG[t][1], TERRAIN_BG[t][2]);
                if (pu && pu->type == UNIT_TYPE_SETTLER) printf("\033[38;2;240;220;50mS");
                else if (pu) printf("\033[38;2;50;150;240mW");
                else if (cid != -1) printf("\033[38;2;255;255;255mC");
                else if (bu) printf("\033[38;2;200;40;40mb");
                else if (alive_camp) printf("\033[38;2;220;120;30mX");
                else printf(" ");
                printf("\033[0m");
            } else {
                char ch;
                if (pu) ch = (pu->type == UNIT_TYPE_SETTLER) ? 'S' : 'W';
                else if (cid != -1) ch = 'C';
                else if (bu) ch = 'b';
                else if (alive_camp) ch = 'X';
                else ch = terrain_char(gs->map.tiles[y][x].terrain);
                putchar(ch);
            }
        }
        printf("\n");
    }
    if (color) {
        printf("\nLegende terrain:\n  ");
        const char *tnames[] = { "Plaine", "Foret", "Montagne", "Eau", "Desert", "Toundra" };
        for (int i = 0; i < TERRAIN_COUNT; i++)
            printf("\033[48;2;%d;%d;%dm  \033[0m=%s  ",
                   TERRAIN_BG[i][0], TERRAIN_BG[i][1], TERRAIN_BG[i][2],
                   tnames[i]);
        printf("\nLegende entites:\n  ");
        printf("\033[38;2;255;255;255mC\033[0m=Ville  ");
        printf("\033[38;2;240;220;50mS\033[0m=Colon  ");
        printf("\033[38;2;50;150;240mW\033[0m=Guerrier  ");
        printf("\033[38;2;200;40;40mb\033[0m=Barbare  ");
        printf("\033[38;2;220;120;30mX\033[0m=Camp barbare\n\n");
    } else {
        printf("\nLegende: P=Plaine F=Foret M=Montagne ~=Eau D=Desert T=Toundra\n");
        printf("         C=Ville S=Colon W=Guerrier b=Barbare X=Camp barbare\n\n");
    }
}

void print_status(GameState *gs) {
    printf("--- Tour %d/%d --- Or: %d --- Science en cours: %d/%d (%s) ---\n",
        gs->turn, gs->turn_max, gs->gold,
        gs->tech.science_stored,
        gs->tech.researching != TECH_ID_NONE ? tech_def(gs->tech.researching)->cost : 0,
        gs->tech.researching != TECH_ID_NONE ? tech_name(gs->tech.researching) : "aucune");
}

void print_units(GameState *gs) {
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

void print_cities(GameState *gs) {
    printf("=== Villes ===\n");
    int found = 0;
    for (City *c = gs->cities; c; c = c->next) {
        printf("  [%d] @ (%d,%d) Pop:%d PV:%d/%d | Nourriture:%d Prod:%d Or:%d Sci:%d\n",
            c->id, c->x, c->y, c->population, c->hp, c->hp_max,
            c->yield_food, c->yield_prod, c->yield_gold, c->yield_science);
        printf("       Batiments: ");
        if (c->buildings == BUILDING_NONE) printf("aucun");
        if (c->buildings & BUILDING_GRANARY) printf("Grenier ");
        if (c->buildings & BUILDING_WORKSHOP) printf("Atelier ");
        if (c->buildings & BUILDING_LIBRARY) printf("Bibliotheque ");
        if (c->buildings & BUILDING_MARKET) printf("Marche ");
        if (c->buildings & BUILDING_BARRACKS) printf("Caserne ");
        if (c->buildings & BUILDING_WALL) printf("Muraille ");
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

void print_tech(GameState *gs) {
    printf("=== Technologies ===\n");
    for (int i = 0; i < TECH_COUNT; i++) {
        const TechDef *d = tech_def(i);
        char buf[32];
        const char *status;
        if (gs->tech.acquired[i]) status = "[OK]";
        else if (gs->tech.researching == i) {
            snprintf(buf, sizeof buf, "[%d/%d]", gs->tech.science_stored, d->cost);
            status = buf;
        }
        else if (tech_available(&gs->tech, i)) status = "[dispo]";
        else status = "[verr.]";
        printf("  %d. %-15s %s  (cout:%d)\n", i, d->name, status, d->cost);
    }
}

void print_help(void) {
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

void print_result(GameState *gs) {
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
