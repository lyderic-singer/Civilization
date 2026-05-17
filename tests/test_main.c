#include "../core/core.h"
#include <stdio.h>

static int passes = 0;
static int failures = 0;

#define ASSERT(cond) \
    if (cond) passes++; \
    else { failures++; printf("  FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); }

static void test_chebyshev(void) {
    ASSERT(chebyshev(0, 0, 0, 0) == 0);
    ASSERT(chebyshev(0, 0, 3, 0) == 3);
    ASSERT(chebyshev(0, 0, 0, 3) == 3);
    ASSERT(chebyshev(0, 0, 3, 3) == 3);
    ASSERT(chebyshev(0, 0, 2, 5) == 5);
    ASSERT(chebyshev(0, 0, 5, 2) == 5);
    ASSERT(chebyshev(5, 5, 2, 2) == 3);
    ASSERT(chebyshev(2, 3, 7, 8) == chebyshev(7, 8, 2, 3));
}

static void test_seed_determinism(void) {
    Seed s = gen_seed("1545998465132");
    int w = 20;
    int h = 15;
    GameState *gs1 = gen_game(w, h, 3, 3, s);
    GameState *gs2 = gen_game(w, h, 100, 3, s);

    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
            ASSERT(gs1->map.tiles[y][x].terrain == gs2->map.tiles[y][x].terrain);

    free_game(gs1);
    free_game(gs2);
}

int main(void) {
    printf("Tests :\n");

    printf("chebyshev... "); test_chebyshev(); printf("ok\n");
    printf("seed determinism... "); test_seed_determinism(); printf("ok\n");

    printf("\n%d passed, %d failed\n", passes, failures);
    return failures > 0 ? 1 : 0;
}
