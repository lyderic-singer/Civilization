#ifndef UNITS_H
#define UNITS_H

#include "core.h"

Unit *unit_create(GameState *gs, UnitType type, UnitOwner owner, int x, int y);
void unit_destroy(GameState *gs, int unit_id);
Unit *unit_at(GameState *gs, int x, int y, UnitOwner owner);
Unit *unit_any_at(GameState *gs, int x, int y);
Unit *unit_by_id(GameState *gs, int id);
int unit_move(GameState *gs, Unit *u, int nx, int ny);
void units_reset_pm(GameState *gs, UnitOwner owner);
int unit_attack_unit(GameState *gs, Unit *attacker, Unit *defender);
int unit_attack_city(GameState *gs, Unit *attacker, City *city);
void units_apply_pm_bonus(GameState *gs);
int  unit_cost(UnitType t);
int  unit_maint(UnitType t);
const char *unit_type_name(UnitType t);
void unit_goto(GameState *gs, Unit *u, int tx, int ty);

#endif