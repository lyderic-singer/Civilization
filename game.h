#ifndef GAME_H
#define GAME_H

#include "structs.h"

/* ── Init / teardown ───────────────────────────────────────── */
GameState *game_init(int width, int height, int turns, int n_barb_camps, Seed seed);
void game_free(GameState *gs);

/* ── Main loop ─────────────────────────────────────────────── */
void game_run(GameState *gs);

/* ── Phase functions ────────────────────────────────────────── */
void game_phase_action(GameState *gs);
void game_phase_production(GameState *gs);
void game_phase_growth(GameState *gs);
void game_phase_barbarians(GameState *gs);
void game_phase_end(GameState *gs);

/* ── Victory / defeat ──────────────────────────────────────── */
GameResult game_check_result(GameState *gs);
int game_compute_score(GameState *gs);

/* ── CLI display helpers ────────────────────────────────────── */
void cli_print_map(GameState *gs);
void cli_print_status(GameState *gs);
void cli_print_units(GameState *gs);
void cli_print_cities(GameState *gs);
void cli_print_tech(GameState *gs);
void cli_print_help(void);
void cli_print_result(GameState *gs);

/* ── Settler colonise ───────────────────────────────────────── */
int game_settler_colonise(GameState *gs, Unit *settler);

/* ── Entry point ────────────────────────────────────────────── */
int game_main(int argc, char **argv);

#endif /* GAME_H */
