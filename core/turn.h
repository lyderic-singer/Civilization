#ifndef TURN_H
#define TURN_H

#include "core.h"

int game_settler_colonise(GameState *gs, Unit *settler);
void apply_gold_phase(GameState *gs);
void apply_science_phase(GameState *gs);
GameResult game_check_result(GameState *gs);
int game_compute_score(GameState *gs);
void game_phase_production(GameState *gs);
void game_phase_end(GameState *gs);

#endif
