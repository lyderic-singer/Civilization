#include "turn.h"

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
