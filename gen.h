#ifndef GEN_H
#define GEN_H

#include <stdint.h>
#include "structs.h"

/* Generate a random seed (160 bits, ETH-address-sized hash) from entropy */
Seed gen_random_seed(void);

/* Build a deterministic seed from a user-supplied uint64 */
Seed gen_seed_from_u64(uint64_t raw);

/* Parse a hex string (up to 40 chars) into a Seed */
Seed gen_seed_from_hex(const char *hex);

/* Generate a full map + place starting city, starting warrior,
   and N barbarian camps.  Fills gs->map, gs->units, gs->cities, gs->camps.
   Uses only the seed stored in gs->seed.  */
void gen_map(GameState *gs);

/* ── internal prng (exposed for testing) ── */
uint64_t gen_lcg_next(uint64_t *state);
int      gen_rand_range(uint64_t *rng, int min_incl, int max_incl);

#endif /* GEN_H */