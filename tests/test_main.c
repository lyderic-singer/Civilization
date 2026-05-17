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

int main(void) {
    printf("Tests :\n");

    printf("chebyshev... "); test_chebyshev(); printf("ok\n");

    printf("\n%d passed, %d failed\n", passes, failures);
    return failures > 0 ? 1 : 0;
}
