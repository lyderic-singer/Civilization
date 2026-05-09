#ifndef GAME_H
#define GAME_H

#include "structs.h"

/* ── Init / teardown ───────────────────────────────────────── */

/* Allocate and initialise a GameState from CLI arguments.
   Calls gen_map internally. */
GameState *game_init(int width, int height, int turns, int n_barb_camps,
                     Seed seed);

/* Free all dynamic memory in gs */
void game_free(GameState *gs);

/* ── Main loop ─────────────────────────────────────────────── */

/* Run the full game (blocks until win/loss) */
void game_run(GameState *gs);

/* ── Phase functions (called internally, exposed for testing) ─ */

/* Phase 1: player action (CLI I/O) */
void game_phase_action(GameState *gs);

/* Phase 2: resource production */
void game_phase_production(GameState *gs);

/* Phase 3: city growth */
void game_phase_growth(GameState *gs);

/* Phase 4: barbarian turn */
void game_phase_barbarians(GameState *gs);

/* Phase 5: end-of-turn checks (victory / defeat) */
void game_phase_end(GameState *gs);

/* ── Victory / defeat ──────────────────────────────────────── */

/* Returns RESULT_ONGOING or the appropriate terminal result */
GameResult game_check_result(GameState *gs);

/* Compute and store score in gs->score */
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

/* Attempt to found a city at unit's position.  Returns 1 on success. */
int game_settler_colonise(GameState *gs, Unit *settler);

/* ── Entry point ────────────────────────────────────────────── */

/* Parse argc/argv, run the game, return exit code */
int game_main(int argc, char **argv);

#endif /* GAME_H */