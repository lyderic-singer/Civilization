#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "core.h"
#include "cli.h"

/* Free all dynamic memory in gs */
void free_game(GameState *gs);
/* Run the full game (blocks until win/loss) */
void game_run(GameState *gs);
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
/* Returns RESULT_ONGOING or the appropriate terminal result */
GameResult game_check_result(GameState *gs);
/* Compute and store score in gs->score */
int game_compute_score(GameState *gs);
/* Attempt to found a city at unit's position.  Returns 1 on success. */
int game_settler_colonise(GameState *gs, Unit *settler);

#endif