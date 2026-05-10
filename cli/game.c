#include "game.h"

/* ═══════════════════════════════════════════════════════════
   FREE
═══════════════════════════════════════════════════════════ */

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
   VICTORY / DEFEAT CHECK
═══════════════════════════════════════════════════════════ */

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

/* ═══════════════════════════════════════════════════════════
   SCORE
═══════════════════════════════════════════════════════════ */

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
   MAIN LOOP
═══════════════════════════════════════════════════════════ */

void game_run(GameState *gs) {
    cities_compute_yields(gs);

    while (gs->result == RESULT_ONGOING) {
        game_phase_action(gs);
        if (gs->result != RESULT_ONGOING) break;

        game_phase_production(gs);
        game_phase_growth(gs);
        game_phase_barbarians(gs);
        game_phase_end(gs);

        if (gs->result == RESULT_ONGOING) {
            gs->turn++;
            cities_compute_yields(gs);
        }
    }

    game_compute_score(gs);
    cli_print_result(gs);
}