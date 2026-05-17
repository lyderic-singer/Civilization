#ifndef SAVE_H
#define SAVE_H

#include "structs.h"

#define SCORES_FILE "scores.txt"
#define SAVES_DIR "saves"

typedef struct {
    char date[32];
    char result[32];
    int score;
    int tour;
} ScoreEntry;

void scores_append(GameState *gs);
void game_save(GameState *gs);
GameState *game_load(const char *path);
int saves_list(char paths[][256], int max);
int scores_read(ScoreEntry *entries, int max);
void format_save_name(const char *filename, char *out, int outlen);

#endif
