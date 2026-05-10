#include "cli/cli.h"

void print_usage(const char *prog) {
    printf("Usage: %s [options]\n", prog);
    printf("  --width  W  / -W W   Largeur de la carte (defaut: %d)\n", DEFAULT_WIDTH);
    printf("  --height H  / -H H   Hauteur de la carte (defaut: %d)\n", DEFAULT_HEIGHT);
    printf("  --seed N    / -s N   Graine (uint64) ou hex 40 chars\n");
    printf("  --turns N   / -t N   Nombre de tours max (defaut: %d)\n", DEFAULT_TURNS);
    printf("  --barbarians N / -b N  Camps barbares (defaut: %d)\n", DEFAULT_BARBARIANS);
    printf("  --mode MODE / -m MODE  Mode: cli (seul supporte)\n");
    printf("  --help\n");
}

int main(int argc, char **argv) {
    int width = DEFAULT_WIDTH;
    int height = DEFAULT_HEIGHT;
    int turns = DEFAULT_TURNS;
    int n_barbs = DEFAULT_BARBARIANS;
    int has_seed = 0;
    Seed seed;
    for (int i = 1; i < argc; i++) {
        if ((strcmp(argv[i], "--width") == 0 || strcmp(argv[i], "-W") == 0) && i+1 < argc) {
            width = atoi(argv[++i]);
        } else if ((strcmp(argv[i], "--height") == 0 || strcmp(argv[i], "-H") == 0) && i+1 < argc) {
            height = atoi(argv[++i]);
        } else if ((strcmp(argv[i], "--turns") == 0 || strcmp(argv[i], "-t") == 0) && i+1 < argc) {
            turns = atoi(argv[++i]);
        } else if ((strcmp(argv[i], "--barbarians") == 0 || strcmp(argv[i], "-b") == 0) && i+1 < argc) {
            n_barbs = atoi(argv[++i]);
        } else if ((strcmp(argv[i], "--mode") == 0 || strcmp(argv[i], "-m") == 0) && i+1 < argc) {
            i++; /* only 'cli' supported */
        } else if ((strcmp(argv[i], "--seed") == 0 || strcmp(argv[i], "-s") == 0) && i+1 < argc) {
            seed = gen_seed(argv[++i]);
            has_seed = 1;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        } else {
            fprintf(stderr, "Parametre invalide: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
    }}
    if (width < 10 || height < 10) {
        fprintf(stderr, "Carte trop petite (minimum 10x10).\n");
        return 1;
    }
    if (turns < 1) {
        fprintf(stderr, "Nombre de tours invalide.\n");
        return 1;
    }
    if (n_barbs < 0) n_barbs = 0;
    if (!has_seed) seed = gen_random_seed();
    GameState *gs = gen_game(width, height, turns, n_barbs, seed);
    game_run(gs);
    free_game(gs);
    return 0;
}
