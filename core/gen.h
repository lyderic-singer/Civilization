#ifndef GEN_H
#define GEN_H

#include "core.h"

Seed gen_random_seed(void);
Seed gen_seed(char *str);
GameState *gen_game(int width, int height, int turns, int n_barb_camps, Seed seed);
void free_game(GameState *gs);

#endif