#ifndef UNITS_H
#define UNITS_H

#include "structs.h"

/* ── Lifecycle ─────────────────────────────────────────────── */

/* Create and register a unit in gs->units; returns pointer */
Unit *unit_create(GameState *gs, UnitType type, UnitOwner owner, int x, int y);

/* Remove and free a unit by id */
void unit_destroy(GameState *gs, int unit_id);

/* Find a unit at (x,y) for a given owner; NULL if none */
Unit *unit_at(GameState *gs, int x, int y, UnitOwner owner);

/* Find any unit at (x,y) regardless of owner */
Unit *unit_any_at(GameState *gs, int x, int y);

/* Find unit by id */
Unit *unit_by_id(GameState *gs, int id);

/* ── Movement ──────────────────────────────────────────────── */

/* Attempt to move unit to (nx,ny).
   Returns 1 on success, 0 if blocked/illegal. */
int unit_move(GameState *gs, Unit *u, int nx, int ny);

/* Reset PM for all units of given owner */
void units_reset_pm(GameState *gs, UnitOwner owner);

/* ── Combat ────────────────────────────────────────────────── */

/* Resolve combat between attacker and defender unit.
   Returns 1 if defender was killed, 0 otherwise. */
int unit_attack_unit(GameState *gs, Unit *attacker, Unit *defender);

/* Attacker attacks a city; returns 1 if city fell, 0 otherwise */
int unit_attack_city(GameState *gs, Unit *attacker, City *city);

/* ── Helpers ───────────────────────────────────────────────── */

int units_count(GameState *gs, UnitOwner owner);
void units_apply_pm_bonus(GameState *gs);

int  unit_cost(UnitType t);
int  unit_maint(UnitType t);
int  unit_hp_max(UnitType t);
int  unit_atk(UnitType t);
int  unit_def(UnitType t);
int  unit_pm_base(UnitType t);
const char *unit_type_name(UnitType t);

#endif /* UNITS_H */
