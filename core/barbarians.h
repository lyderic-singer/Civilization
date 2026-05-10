#ifndef BARBARIANS_H
#define BARBARIANS_H

#include <stdio.h>
#include <stdlib.h>

#include "structs.h"
#include "utils.h"
#include "units.h"
#include "cities.h"
#include "config.h"

void barbarians_turn(GameState *gs);
void barbarians_spawn(GameState *gs);
void barbarians_move_and_attack(GameState *gs);

#endif