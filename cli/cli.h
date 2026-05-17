#ifndef GAME_H
#define GAME_H

#include "../core/core.h"
#include "../core/turn.h"

void game_run(GameState *gs);
int cli_choose_start(char *out_path);

void print_map(GameState *gs);
void print_status(GameState *gs);
void print_units(GameState *gs);
void print_cities(GameState *gs);
void print_tech(GameState *gs);
void print_help(void);
void print_result(GameState *gs);

#endif