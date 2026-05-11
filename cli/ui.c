#include "ui.h"

char terrain_char(TerrainType t) {
    switch (t) {
        case TERRAIN_PLAIN: return 'P';
        case TERRAIN_FOREST: return 'F';
        case TERRAIN_MOUNTAIN: return 'M';
        case TERRAIN_WATER: return '~';
        case TERRAIN_DESERT: return 'D';
        case TERRAIN_TUNDRA: return 'T';
        default: return '?';
    }
}

void print_map(GameState *gs) {
    int W = gs->map.width;
    int H = gs->map.height;
    printf("   ");
    for (int x = 0; x < W; x++) {
        if (x % 10 == 0) printf("|");
        else printf("%d", x % 10);
    }
    printf("\n");
    for (int y = 0; y < H; y++) {
        printf("%2d ", y);
        for (int x = 0; x < W; x++) {
            Unit *pu = unit_at(gs, x, y, UNIT_OWNER_PLAYER);
            Unit *bu = unit_at(gs, x, y, UNIT_OWNER_BARB);
            int cid  = gs->map.tiles[y][x].city_id;
            int pid  = gs->map.tiles[y][x].camp_id;
            char ch;
            if (pu) {
                ch = (pu->type == UNIT_TYPE_SETTLER) ? 'S' : 'W';
            } else if (cid != -1) {
                ch = 'C';
            } else if (bu) {
                ch = 'b';
            } else if (pid != -1) {
                int alive = 0;
                for (Camp *c = gs->camps; c; c = c->next)
                    if (c->id == pid && c->alive) { alive = 1; break; }
                ch = alive ? 'X' : terrain_char(gs->map.tiles[y][x].terrain);
            } else {
                ch = terrain_char(gs->map.tiles[y][x].terrain);
            }
            putchar(ch);
        }
        printf("\n");
    }
    printf("\nLegende: P=Plaine F=Foret M=Montagne ~=Eau D=Desert T=Toundra\n");
    printf("         C=Ville S=Colon W=Guerrier b=Barbare X=Camp barbare\n\n");
}

void print_status(GameState *gs) {
    printf("--- Tour %d/%d --- Or: %d --- Science en cours: %d/%d (%s) ---\n",
        gs->turn, gs->turn_max, gs->gold,
        gs->tech.science_stored,
        gs->tech.researching != TECH_ID_NONE ? tech_def(gs->tech.researching)->cost : 0,
        gs->tech.researching != TECH_ID_NONE ? tech_name(gs->tech.researching) : "aucune");
}

void print_units(GameState *gs) {
    printf("=== Unites du joueur ===\n");
    int found = 0;
    for (Unit *u = gs->units; u; u = u->next) {
        if (u->owner != UNIT_OWNER_PLAYER) continue;
        printf("  [%d] %s @ (%d,%d) PV:%d/%d ATK:%d DEF:%d PM:%d/%d\n",
            u->id, unit_type_name(u->type), u->x, u->y,
            u->hp, u->hp_max, u->attack, u->defense, u->pm_left, u->pm_max);
        found = 1;
    }
    if (!found) printf("  (aucune unite)\n");
}

void print_cities(GameState *gs) {
    printf("=== Villes ===\n");
    int found = 0;
    for (City *c = gs->cities; c; c = c->next) {
        printf("  [%d] @ (%d,%d) Pop:%d PV:%d/%d | Nourriture:%d Prod:%d Or:%d Sci:%d\n",
            c->id, c->x, c->y, c->population, c->hp, c->hp_max,
            c->yield_food, c->yield_prod, c->yield_gold, c->yield_science);
        printf("       Batiments: ");
        if (c->buildings == BUILDING_NONE) printf("aucun");
        if (c->buildings & BUILDING_GRANARY) printf("Grenier ");
        if (c->buildings & BUILDING_WORKSHOP) printf("Atelier ");
        if (c->buildings & BUILDING_LIBRARY) printf("Bibliotheque ");
        if (c->buildings & BUILDING_MARKET) printf("Marche ");
        if (c->buildings & BUILDING_BARRACKS) printf("Caserne ");
        if (c->buildings & BUILDING_WALL) printf("Muraille ");
        printf("\n");
        if (c->project_type != PROJECT_NONE) {
            const char *proj_name = (c->project_type == PROJECT_BUILDING)
                ? building_name(c->project_id)
                : unit_type_name((UnitType)c->project_id);
            int proj_cost = city_project_cost(gs, c);
            printf("       Projet: %s [%d/%d]\n", proj_name, c->prod_stock, proj_cost);
        }
        found = 1;
    }
    if (!found) printf("  (aucune ville)\n");
}

void print_tech(GameState *gs) {
    printf("=== Technologies ===\n");
    for (int i = 0; i < TECH_COUNT; i++) {
        const TechDef *d = tech_def(i);
        const char *status;
        if (gs->tech.acquired[i]) status = "[OK]";
        else if (gs->tech.researching == i) {
            char buf[32];
            snprintf(buf, sizeof buf, "[%d/%d]", gs->tech.science_stored, d->cost);
            status = buf;
        }
        else if (tech_available(&gs->tech, i)) status = "[dispo]";
        else {
            status = "[verr.]";
            printf("  %d. %-15s %s  (cout:%d)\n", i, d->name, status, d->cost);
    }}
}

void print_help(void) {
    printf("\n=== Commandes ===\n");
    printf("  m           Afficher la carte\n");
    printf("  s           Afficher le statut\n");
    printf("  u           Lister les unites\n");
    printf("  c           Lister les villes\n");
    printf("  t           Arbre technologique\n");
    printf("  move <id> <dx> <dy>   Deplacer une unite\n");
    printf("  goto <id> <x> <y>    Deplacer une unite vers (x,y) (greedy)\n");
    printf("  col <id>             Coloniser (settler uniquement)\n");
    printf("  proj <city_id> <type> <id>   Definir projet (type: u=unite b=batiment)\n");
    printf("  research <tech_id>   Lancer recherche\n");
    printf("  end                  Passer au tour suivant\n");
    printf("  help                 Afficher cette aide\n");
    printf("  quit                 Quitter\n\n");
}

void print_result(GameState *gs) {
    printf("\n");
    switch (gs->result) {
        case RESULT_WIN_SCI:
            printf("*** VICTOIRE SCIENTIFIQUE ***\n");
            printf("Vous avez acquis toutes les technologies!\n");
            break;
        case RESULT_WIN_TERR:
            printf("*** VICTOIRE TERRITORIALE ***\n");
            printf("Vous avez controle 10 villes pendant 5 tours consecutifs!\n");
            break;
        case RESULT_LOSS_NOCITY:
            printf("*** DEFAITE - Toutes vos villes ont ete detruites ***\n");
            break;
        case RESULT_LOSS_TURNS:
            printf("*** DEFAITE - Nombre de tours maximum atteint (%d) ***\n", gs->turn_max);
            break;
        case RESULT_LOSS_PROD:
            printf("*** DEFAITE - Production nulle pendant %d tours consecutifs ***\n",
                   GOLD_BANKRUPT_PROD_HALVE_TURNS);
            break;
        default: break;
    }
    printf("Score final: %d\n\n", gs->score);
}
