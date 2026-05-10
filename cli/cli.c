
#include "cli.h"

const int MOVE_COSTS[TERRAIN_COUNT] = {
    TERRAIN_PLAIN_COST, TERRAIN_FOREST_COST, TERRAIN_MOUNTAIN_COST,
    TERRAIN_WATER_COST, TERRAIN_DESERT_COST, TERRAIN_TUNDRA_COST
};

int game_settler_colonise(GameState *gs, Unit *settler) {
    if (settler->type != UNIT_TYPE_SETTLER || settler->owner != UNIT_OWNER_PLAYER) {
        printf("Seul un Colon peut coloniser.\n");
        return 0;
    }
    int x = settler->x, y = settler->y;
    TerrainType t = gs->map.tiles[y][x].terrain;
    if (t == TERRAIN_WATER) {
        printf("Impossible de fonder une ville sur l'eau.\n");
        return 0;
    }
    if (gs->map.tiles[y][x].city_id != -1 || gs->map.tiles[y][x].camp_id != -1) {
        printf("Tuile deja occupee.\n");
        return 0;
    }
    for (City *c = gs->cities; c; c = c->next) {
        int dx = c->x - x; if (dx < 0) dx = -dx;
        int dy = c->y - y; if (dy < 0) dy = -dy;
        int d = dx > dy ? dx : dy;
        if (d < CITY_MIN_DISTANCE) {
            printf("Trop proche d'une ville existante (distance Chebyshev: %d < %d).\n",
                   d, CITY_MIN_DISTANCE);
            return 0;
    }}
    int sid = settler->id;
    unit_destroy(gs, sid);
    city_create(gs, x, y, gs->turn);
    printf("[Fondation] Nouvelle ville en (%d,%d)!\n", x, y);
    return 1;
}

void unit_goto(GameState *gs, Unit *u, int tx, int ty) {
    while (u->pm_left > 0) {
        if (u->x == tx && u->y == ty) break;
        int best_dx = 0, best_dy = 0;
        int best_d = chebyshev(u->x, u->y, tx, ty);
        int best_cost = 999;
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (dx == 0 && dy == 0) continue;
                int nx = u->x + dx, ny = u->y + dy;
                if (nx < 0 || nx >= gs->map.width || ny < 0 || ny >= gs->map.height) continue;
                TerrainType t = gs->map.tiles[ny][nx].terrain;
                int cost = MOVE_COSTS[t];
                if (cost == 0 || cost > u->pm_left) continue;
                Unit *occ = unit_at(gs, nx, ny, UNIT_OWNER_PLAYER);
                if (occ && occ->id != u->id) continue;
                int d = chebyshev(nx, ny, tx, ty);
                if (d < best_d || (d == best_d && cost < best_cost)) {
                    best_d = d; best_cost = cost;
                    best_dx = dx; best_dy = dy;
        }}}
        if (best_dx == 0 && best_dy == 0) break;
        int nx = u->x + best_dx, ny = u->y + best_dy;
        if (!unit_move(gs, u, nx, ny)) break;
        if (!unit_by_id(gs, u->id)) break;
    }
}

void apply_gold_phase(GameState *gs) {
    int gold_income = 0;
    for (City *c = gs->cities; c; c = c->next)
        gold_income += c->yield_gold;
    int unit_maint_total = 0;
    for (Unit *u = gs->units; u; u = u->next)
        if (u->owner == UNIT_OWNER_PLAYER) unit_maint_total += u->maint;
    int building_maint_total = cities_total_building_maint(gs);
    int expenses = unit_maint_total + building_maint_total;
    gs->gold += gold_income - expenses;
    if (gs->gold < 0) {
        printf("[Banqueroute] Or < 0 (%d). ", gs->gold);
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

void apply_science_phase(GameState *gs) {
    int total_sci = 0;
    for (City *c = gs->cities; c; c = c->next)
        total_sci += c->yield_science;
    if (gs->tech.researching != TECH_ID_NONE) {
        int completed = tech_add_science(&gs->tech, total_sci);
        if (completed != TECH_ID_NONE) {
            tech_apply_effects(gs);
    }}
}

GameResult game_check_result(GameState *gs) {
    if (tech_all_acquired(&gs->tech))
        return RESULT_WIN_SCI;
    if (cities_count(gs) >= VICTORY_TERRITORIAL_CITIES) {
        gs->territorial_consecutive_turns++;
        if (gs->territorial_consecutive_turns >= VICTORY_TERRITORIAL_TURNS)
            return RESULT_WIN_TERR;
    } else {
        gs->territorial_consecutive_turns = 0;
    }
    if (cities_count(gs) == 0)
        return RESULT_LOSS_NOCITY;
    if (gs->turn > gs->turn_max)
        return RESULT_LOSS_TURNS;
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
        game_compute_score(gs);
        print_result(gs);
        free_game(gs);
        exit(0);
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

void game_phase_production(GameState *gs) {
    cities_compute_yields(gs);
    cities_produce(gs);
    apply_gold_phase(gs);
    apply_science_phase(gs);
}

void game_phase_end(GameState *gs) {
    cities_heal(gs);
    cities_reset_damage_flag(gs);
    gs->result = game_check_result(gs);
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
    print_result(gs);
}
