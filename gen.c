#include "gen.h"
#include "config.h"
#include "structs.h"
#include "units.h"
#include "cities.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

/* ═══════════════════════════════════════════════════════════
   LCG PRNG  (fast, deterministic, seed-driven)
   Knuth multiplicative LCG:
   x_{n+1} = x_n * 6364136223846793005 + 1442695040888963407
═══════════════════════════════════════════════════════════ */

uint64_t gen_lcg_next(uint64_t *state) {
    *state = *state * UINT64_C(6364136223846793005)
           + UINT64_C(1442695040888963407);
    return *state;
}

int gen_rand_range(uint64_t *rng, int min_incl, int max_incl) {
    if (min_incl >= max_incl) return min_incl;
    uint64_t r = gen_lcg_next(rng);
    return min_incl + (int)(r % (uint64_t)(max_incl - min_incl + 1));
}

/* ═══════════════════════════════════════════════════════════
   SEED GENERATION
═══════════════════════════════════════════════════════════ */

/* FNV-1a 64 on arbitrary bytes */
static uint64_t fnv1a_64(const void *data, size_t len) {
    const uint8_t *p = (const uint8_t *)data;
    uint64_t hash = UINT64_C(14695981039346656037);
    for (size_t i = 0; i < len; i++) {
        hash ^= p[i];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static void seed_fill_hex(Seed *s) {
    uint8_t raw[20];
    for (int i = 0; i < 8; i++) raw[i]    = (uint8_t)(s->parts[0] >> (56 - 8*i));
    for (int i = 0; i < 8; i++) raw[8+i]  = (uint8_t)(s->parts[1] >> (56 - 8*i));
    for (int i = 0; i < 4; i++) raw[16+i] = (uint8_t)(s->parts[2] >> (24 - 8*i));
    for (int i = 0; i < 20; i++)
        snprintf(s->hex + 2*i, 3, "%02x", raw[i]);
    s->hex[SEED_HEX_LEN] = '\0';
}

Seed gen_random_seed(void) {
    Seed s;
    uint64_t t = (uint64_t)time(NULL) ^ (uint64_t)clock();
    uint64_t e = fnv1a_64(&t, sizeof(t));
    s.parts[0] = fnv1a_64(&e, sizeof(e));
    s.parts[1] = fnv1a_64(&s.parts[0], sizeof(s.parts[0]));
    s.parts[2] = fnv1a_64(&s.parts[1], sizeof(s.parts[1]));
    seed_fill_hex(&s);
    return s;
}

Seed gen_seed_from_u64(uint64_t raw) {
    Seed s;
    s.parts[0] = fnv1a_64(&raw, sizeof raw);
    s.parts[1] = fnv1a_64(&s.parts[0], sizeof s.parts[0]);
    s.parts[2] = fnv1a_64(&s.parts[1], sizeof s.parts[1]);
    seed_fill_hex(&s);
    return s;
}

Seed gen_seed_from_hex(const char *hex) {
    Seed s;
    memset(&s, 0, sizeof s);
    size_t len = strlen(hex);
    if (len > SEED_HEX_LEN) len = SEED_HEX_LEN;
    uint8_t raw[20] = {0};
    for (size_t i = 0; i + 1 < len; i += 2) {
        unsigned v;
        sscanf(hex + i, "%02x", &v);
        raw[i/2] = (uint8_t)v;
    }
    for (int i = 0; i < 8; i++) s.parts[0] |= ((uint64_t)raw[i])    << (56 - 8*i);
    for (int i = 0; i < 8; i++) s.parts[1] |= ((uint64_t)raw[8+i])  << (56 - 8*i);
    for (int i = 0; i < 4; i++) s.parts[2] |= ((uint64_t)raw[16+i]) << (24 - 8*i);
    seed_fill_hex(&s);
    return s;
}
