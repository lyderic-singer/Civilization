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

/* ═══════════════════════════════════════════════════════════
   GREEDY MOVE TOWARD TARGET
═══════════════════════════════════════════════════════════ */

static const int TERRAIN_COSTS[TERRAIN_COUNT] = {
    TERRAIN_PLAIN_COST,
    TERRAIN_FOREST_COST,
    TERRAIN_MOUNTAIN_COST,
    TERRAIN_WATER_COST,
    TERRAIN_DESERT_COST,
    TERRAIN_TUNDRA_COST
};

static void barb_move_toward(GameState *gs, Unit *barb, int tx, int ty) {
    int best_dx = 0, best_dy = 0;
    int best_dist = chebyshev(barb->x, barb->y, tx, ty);
    int best_cost = 999;

    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            int nx = barb->x + dx;
            int ny = barb->y + dy;
            if (nx < 0 || nx >= gs->map.width || ny < 0 || ny >= gs->map.height) continue;

            TerrainType t = gs->map.tiles[ny][nx].terrain;
            int cost = TERRAIN_COSTS[t];
            if (cost == 0) continue;
            if (cost > barb->pm_left) continue;

            Unit *occ = unit_at(gs, nx, ny, UNIT_OWNER_BARB);
            if (occ && occ->id != barb->id) continue;

            int d = chebyshev(nx, ny, tx, ty);
            if (d < best_dist ||
                (d == best_dist && cost < best_cost) ||
                (d == best_dist && cost == best_cost &&
                 (dy < best_dy || (dy == best_dy && dx < best_dx)))) {
                best_dist = d;
                best_cost = cost;
                best_dx   = dx;
                best_dy   = dy;
            }
        }
    }

    if (best_dist == chebyshev(barb->x, barb->y, tx, ty) && best_dx == 0 && best_dy == 0)
        return;
    if (best_dx == 0 && best_dy == 0) return;

    int nx = barb->x + best_dx;
    int ny = barb->y + best_dy;

    /* Check for player unit at destination → attack */
    Unit *player_unit = unit_at(gs, nx, ny, UNIT_OWNER_PLAYER);
    if (player_unit) {
        printf("[Barbare %d] Attaque unite joueur id=%d\n", barb->id, player_unit->id);
        unit_attack_unit(gs, barb, player_unit);
        return;
    }

    /* Check for player city at destination → attack city */
    int city_id = gs->map.tiles[ny][nx].city_id;
    if (city_id != -1) {
        City *city = city_by_id(gs, city_id);
        if (city) {
            printf("[Barbare %d] Attaque ville id=%d\n", barb->id, city_id);
            unit_attack_city(gs, barb, city);
            return;
        }
    }

    barb->pm_left -= best_cost;
    barb->x = nx;
    barb->y = ny;
}

/* ═══════════════════════════════════════════════════════════
   SPAWN
═══════════════════════════════════════════════════════════ */

void barbarians_spawn(GameState *gs) {
    int cap = gs->n_barbarian_camps_initial * BARB_UNIT_CAP_FACTOR;
    if (gs->barb_unit_count >= cap) return;

    for (Camp *camp = gs->camps; camp; camp = camp->next) {
        if (!camp->alive) continue;
        if (gs->barb_unit_count >= cap) break;
        if (unit_any_at(gs, camp->x, camp->y)) continue;

        unit_create(gs, UNIT_TYPE_BARB, UNIT_OWNER_BARB, camp->x, camp->y);
        printf("[Camp barbare %d] Spawn guerrier barbare\n", camp->id);
    }
}

/* ═══════════════════════════════════════════════════════════
   MOVE AND ATTACK
═══════════════════════════════════════════════════════════ */

void barbarians_move_and_attack(GameState *gs) {
    int barb_ids[256];
    int n_barbs = 0;
    for (Unit *u = gs->units; u; u = u->next)
        if (u->owner == UNIT_OWNER_BARB && n_barbs < 256)
            barb_ids[n_barbs++] = u->id;

    for (int i = 0; i < n_barbs; i++) {
        Unit *barb = unit_by_id(gs, barb_ids[i]);
        if (!barb) continue;

        barb->pm_left = barb->pm_max;

        int tx, ty;
        if (!find_nearest_target(gs, barb->x, barb->y, &tx, &ty)) continue;

        int dist = chebyshev(barb->x, barb->y, tx, ty);

        if (dist == 1) {
            Unit *player_unit = unit_at(gs, tx, ty, UNIT_OWNER_PLAYER);
            if (player_unit) {
                printf("[Barbare %d] Attaque directe unite joueur id=%d\n",
                       barb->id, player_unit->id);
                unit_attack_unit(gs, barb, player_unit);
                continue;
            }
            int cid = gs->map.tiles[ty][tx].city_id;
            if (cid != -1) {
                City *city = city_by_id(gs, cid);
                if (city) {
                    printf("[Barbare %d] Attaque directe ville id=%d\n", barb->id, cid);
                    unit_attack_city(gs, barb, city);
                    continue;
                }
            }
        }

        while (barb->pm_left > 0) {
            int old_x = barb->x, old_y = barb->y;
            barb_move_toward(gs, barb, tx, ty);
            if (!unit_by_id(gs, barb_ids[i])) break;
            if (barb->x == old_x && barb->y == old_y) break;
            if (chebyshev(barb->x, barb->y, tx, ty) <= 1) break;
        }
    }
}

/* ═══════════════════════════════════════════════════════════
   FULL BARBARIAN PHASE
═══════════════════════════════════════════════════════════ */

void barbarians_turn(GameState *gs) {
    barbarians_spawn(gs);
    barbarians_move_and_attack(gs);
}
