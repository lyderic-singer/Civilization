#ifndef UTILS_H
#define UTILS_H

#include "core.h"

int chebyshev(int ax, int ay, int bx, int by);
Seed gen_random_seed(void);
Seed gen_seed(char *str);
uint64_t mix(uint64_t x);
int gen_rand_range(uint64_t rng, int min_incl, int max_incl);
float *gen_heightmap(int W, int H, uint64_t rng);

#endif
