#ifndef CLI_H
#define CLI_H

#include "../core/core.h"

void print_map(GameState *gs);
void print_status(GameState *gs);
void print_units(GameState *gs);
void print_cities(GameState *gs);
void print_tech(GameState *gs);
void print_help(void);
void print_result(GameState *gs);

#endif