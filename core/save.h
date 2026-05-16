#ifndef SAVE_H
#define SAVE_H

#include "structs.h"

#define SCORES_FILE "scores.txt"
#define SAVES_DIR "saves"

void scores_append(GameState *gs);
void game_save(GameState *gs);

#endif
