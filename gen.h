#ifndef GEN_H
#define GEN_H

#include <stdint.h>
#include "structs.h"
#include "utils.h"

Seed gen_random_seed(void);
Seed gen_seed(char *str);
void gen_map(GameState *gs);

#endif