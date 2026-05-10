#ifndef PHASE_H
#define PHASE_H

#include "core.h"

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

/* Attempt to found a city at unit's position. Returns 1 on success. */
int game_settler_colonise(GameState *gs, Unit *settler);

#endif
