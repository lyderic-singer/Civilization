#include "utils.h"

int chebyshev(int ax, int ay, int bx, int by) {
    int dx = ax - bx; if (dx < 0) dx = -dx;
    int dy = ay - by; if (dy < 0) dy = -dy;
    return dx > dy ? dx : dy;
}

void compute_hex(Seed *s) {
    static const char *h = "0123456789abcdef";
    uint64_t v = s->value;
    for (int i = SEED_HEX_LEN - 1; i >= 0; --i) {
        s->hex[i] = h[v & 0xF];
        v >>= 4;
    }
    s->hex[SEED_HEX_LEN] = '\0';
}

Seed gen_seed(char *str) {
    Seed s;
    uint64_t v;
    uint64_t hash = 14695981039346656037ULL;
    if (str && str[0] == '0' && str[1] == 'x' && strlen(str) == SEED_HEX_LEN + 2) {
        v = strtoull(str + 2, NULL, 16);
    } else {
        while (str && *str) {
            hash ^= (uint8_t)*str++;
            hash *= 1099511628211ULL;
        }
        v = hash;
    }
    s.value = v;
    compute_hex(&s);
    return s;
}

Seed gen_random_seed(void) {
    Seed s;
    uint64_t x = (uint64_t)time(NULL);
    x ^= (uint64_t)(uintptr_t)&s;
    x ^= (uint64_t)clock();
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    s.value = x;
    compute_hex(&s);
    return s;
}

uint64_t mix(uint64_t x) {
    return x * UINT64_C(6364136223846793005) + UINT64_C(1442695040888963407);
}

int gen_rand_range(uint64_t rng, int min_incl, int max_incl) {
    if (min_incl >= max_incl) return min_incl;
    uint64_t r = mix(rng);
    return min_incl + (int)(r % (uint64_t)(max_incl - min_incl + 1));
}

float *gen_heightmap(int W, int H, uint64_t rng) {
    float *hmap = malloc(W * H * sizeof(float));
    if (!hmap) { perror("malloc"); exit(1); }
    float noise_table[256];
    for (int i = 0; i < 256; i++) {
        rng = mix(rng);
        noise_table[i] = (float)(mix(rng) & 0xFFFF) / 65535.0f;
    }
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            float v = 0.0f;
            float amp = 1.0f, freq = 1.0f, total_amp = 0.0f;
            for (int o = 0; o < 4; o++) {
                int ix = (int)(x * freq) & 255;
                int iy = (int)(y * freq) & 255;
                int idx = (ix * 31 + iy * 17 + o * 97) & 255;
                v += noise_table[idx] * amp;
                total_amp += amp;
                amp  *= 0.5f;
                freq *= 2.0f;
            }
            hmap[y * W + x] = v / total_amp;
        }}
    return hmap;
}
