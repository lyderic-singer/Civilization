#ifndef CITIES_H
#define CITIES_H

#include "structs.h"

/* ── Lifecycle ─────────────────────────────────────────────── */
City *city_create(GameState *gs, int x, int y, int founded_turn);
void city_raze(GameState *gs, int city_id);
City *city_by_id(GameState *gs, int id);
City *city_at(GameState *gs, int x, int y);
int cities_count(GameState *gs);

/* ── Tile exploitation ─────────────────────────────────────── */
void cities_compute_yields(GameState *gs);

/* ── Production phase ─────────────────────────────────────── */
void cities_produce(GameState *gs);

/* ── Growth phase ──────────────────────────────────────────── */
void cities_grow(GameState *gs);

/* ── Heal phase ────────────────────────────────────────────── */
void cities_heal(GameState *gs);
void cities_reset_damage_flag(GameState *gs);

/* ── HP helpers ────────────────────────────────────────────── */
void city_recompute_hp(City *c);

/* ── Gold maintenance ─────────────────────────────────────── */
int cities_total_building_maint(GameState *gs);

/* ── Building helpers ─────────────────────────────────────── */
int city_can_build(GameState *gs, City *c, int building_flag);
int building_cost(int flag);
int building_maint(int flag);
const char *building_name(int flag);

/* ── Project helpers ──────────────────────────────────────── */
void city_set_project(City *c, int project_type, int project_id);
void city_cancel_project(City *c);
int  city_project_cost(GameState *gs, City *c);
int  city_total_pop(GameState *gs);
int  city_exploit_radius(int population);

#endif /* CITIES_H */
