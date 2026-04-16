#include "barbarians.h"
#include "units.h"
#include "cities.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>

/* ═══════════════════════════════════════════════════════════
   HELPERS
═══════════════════════════════════════════════════════════ */

static int chebyshev(int ax, int ay, int bx, int by) {
    int dx = ax - bx; if (dx < 0) dx = -dx;
    int dy = ay - by; if (dy < 0) dy = -dy;
    return dx > dy ? dx : dy;
}

/* Find the nearest player unit OR city to (bx,by).
   Priority: unit before city when equidistant.
   Returns 1 if a target was found, 0 if the player has nothing left. */
static int find_nearest_target(GameState *gs, int bx, int by, int *tx, int *ty) {
    int best_dist    = 999999;
    int best_is_unit = 0;
    int best_id      = 999999;
    int best_x = -1, best_y = -1;

    for (Unit *u = gs->units; u; u = u->next) {
        if (u->owner != UNIT_OWNER_PLAYER) continue;
        int d = chebyshev(bx, by, u->x, u->y);
        if (d < best_dist || (d == best_dist && (1 > best_is_unit || u->id < best_id))) {
            best_dist    = d;
            best_is_unit = 1;
            best_id      = u->id;
            best_x       = u->x;
            best_y       = u->y;
        }
    }

    for (City *c = gs->cities; c; c = c->next) {
        int d = chebyshev(bx, by, c->x, c->y);
        if (d < best_dist || (d == best_dist && !best_is_unit && c->id < best_id)) {
            best_dist    = d;
            best_is_unit = 0;
            best_id      = c->id;
            best_x       = c->x;
            best_y       = c->y;
        }
    }

    if (best_x == -1) return 0;
    *tx = best_x;
    *ty = best_y;
    return 1;
}
