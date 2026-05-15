#ifndef GEN_H
#define GEN_H

#include "structs.h"

GameState *gen_game(int width, int height, int turns, int n_barb_camps, Seed seed);
void free_game(GameState *gs);

#endif