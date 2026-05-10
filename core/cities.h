#ifndef CITIES_H
#define CITIES_H

#include "core.h"

/* create city and set tule */
City *city_create(GameState *gs, int x, int y, int founded_turn);
/* Raze (destroy) a city by id; updates map tile */
void city_raze(GameState *gs, int city_id);
/* Find city by id */
City *city_by_id(GameState *gs, int id);
/* Find player city at (x,y); NULL if none */
City *city_at(GameState *gs, int x, int y);
/* Count active cities */
int cities_count(GameState *gs);
/* Recompute yield_food/prod/gold/science for every city */
void cities_compute_yields(GameState *gs);
/* Advance each city's project by its production yield */
void cities_produce(GameState *gs);
/* Apply food income, check for growth / famine for all cities. */
void cities_grow(GameState *gs);
/* Restore HP for cities that took no damage this turn */
void cities_heal(GameState *gs);
/* Reset took_damage_this_turn flag at start of turn */
void cities_reset_damage_flag(GameState *gs);
/* recompute city hp */
void city_recompute_hp(City *c);
/* Returns total gold maintenance for all city buildings */
int cities_total_building_maint(GameState *gs);
/* Can the city build this building */
int city_can_build(GameState *gs, City *c, int building_flag);
/* Cost to build a building */
int building_cost(int flag);
int building_maint(int flag);
const char *building_name(int flag);
/* Set city project (PROJECT_UNIT or PROJECT_BUILDING + id) */
void city_set_project(City *c, int project_type, int project_id);
/* Cancel project (does not refund prod_stock) */
void city_cancel_project(City *c);
/* Production cost for current project */
int city_project_cost(GameState *gs, City *c);
/* Total production yield for a city (for display) */
int city_total_pop(GameState *gs);
/* Returns exploitation radius for given population */
int city_exploit_radius(int population);

#endif