#ifndef CLI_H
#define CLI_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../core/core.h"

void cli_print_map(GameState *gs);
void cli_print_status(GameState *gs);
void cli_print_units(GameState *gs);
void cli_print_cities(GameState *gs);
void cli_print_tech(GameState *gs);
void cli_print_help(void);
void cli_print_result(GameState *gs);

#endif