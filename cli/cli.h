#ifndef CLI_H
#define CLI_H

#include "core.h"

void cli_print_map(GameState *gs);
void cli_print_status(GameState *gs);
void cli_print_units(GameState *gs);
void cli_print_cities(GameState *gs);
void cli_print_tech(GameState *gs);
void cli_print_help(void);
void cli_print_result(GameState *gs);

#endif