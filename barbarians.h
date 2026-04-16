#ifndef BARBARIANS_H
#define BARBARIANS_H

#include "structs.h"

/* Execute the full barbarian phase for one turn */
void barbarians_turn(GameState *gs);

/* Spawn at most one barb warrior per alive camp */
void barbarians_spawn(GameState *gs);

/* Move and attack for all barb units */
void barbarians_move_and_attack(GameState *gs);

#endif /* BARBARIANS_H */
