#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "core.h"
#include "cli.h"
#include "phase.h"

/* Free all dynamic memory in gs */
void free_game(GameState *gs);

/* Run the full game (blocks until win/loss) */
void game_run(GameState *gs);

/* Returns RESULT_ONGOING or the appropriate terminal result */
GameResult game_check_result(GameState *gs);

/* Compute and store score in gs->score */
int game_compute_score(GameState *gs);

#endif