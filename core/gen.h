#ifndef GEN_H
#define GEN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#include <stdint.h>
#include "structs.h"
#include "utils.h"
#include "config.h"
#include "units.h"
#include "tech.h"
#include "cities.h"

Seed gen_random_seed(void);
Seed gen_seed(char *str);
GameState *gen_game(int width, int height, int turns, int n_barb_camps, Seed seed);

#endif