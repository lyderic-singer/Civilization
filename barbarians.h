#ifndef BARBARIANS_H
#define BARBARIANS_H

#include "structs.h"

/* Execute the full barbarian phase for one turn:
   1. Spawn warriors from alive camps (if cap not reached)
   2. Move each barb unit toward nearest player unit / city
   3. Attack if adjacent to a player unit or city */
void barbarians_turn(GameState *gs);

/* Spawn at most one barb warrior per alive camp (if tile is free and cap ok) */
void barbarians_spawn(GameState *gs);

/* Move and attack for all barb units */
void barbarians_move_and_attack(GameState *gs);

#endif /* BARBARIANS_H */