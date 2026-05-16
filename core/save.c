#include "save.h"
#include "core.h"
#include "turn.h"
#include <stdio.h>
#include <time.h>

static const char *result_str(GameResult r) {
    switch (r) {
        case RESULT_WIN_SCI: return "VICTOIRE_SCI";
        case RESULT_WIN_TERR: return "VICTOIRE_TERR";
        case RESULT_LOSS_NOCITY: return "DEFAITE_NOCITY";
        case RESULT_LOSS_TURNS: return "DEFAITE_TURNS";
        case RESULT_LOSS_PROD: return "DEFAITE_PROD";
        default: return "INCONNU";
    }
}

void scores_append(GameState *gs) {
    FILE *f = fopen(SCORES_FILE, "a");
    if (!f) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char date[16];
    strftime(date, sizeof(date), "%Y-%m-%d", t);

    fprintf(f, "%s %s %d tour:%d seed:%s\n",
            date,
            result_str(gs->result),
            game_compute_score(gs),
            gs->turn,
            gs->seed.hex);
    fclose(f);
}
