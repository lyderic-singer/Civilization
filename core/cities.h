#ifndef CITIES_H
#define CITIES_H

#include "structs.h"

typedef struct {
    int flag;
    const char *name;
    int cost;
    int maint;
} BuildingInfo;

City *city_create(GameState *gs, int x, int y, int founded_turn);
void city_raze(GameState *gs, int city_id);
City *city_by_id(GameState *gs, int id);
City *city_at(GameState *gs, int x, int y);
int cities_count(GameState *gs);
void cities_compute_yields(GameState *gs);
void cities_produce(GameState *gs);
void cities_grow(GameState *gs);
void cities_heal(GameState *gs);
void cities_reset_damage_flag(GameState *gs);
int cities_total_building_maint(GameState *gs);
int city_can_build(GameState *gs, City *c, int building_flag);
int city_growth_threshold(City *c);
int city_can_produce_unit(GameState *gs, City *c, UnitType ut);
const char *building_name(int flag);
void city_set_project(City *c, int project_type, int project_id);
int city_project_cost(GameState *gs, City *c);
int city_total_pop(GameState *gs);

#endif
