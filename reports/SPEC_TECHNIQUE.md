# Spécification technique — Early Civilization

Projet PP2I - 1A Telecom Nancy 2025-2026  
Sujet : Civilization

**Membres du groupe :**  
Lydéric SINGER, Jules GOETZ

---

## Sommaire

- [Architecture des modules](#architecture-des-modules)
- [Structures de données](#structures-de-données)
- [Algorithmes](#algorithmes)
- [Extension EXT-4](#extension-ext-4)
- [Simplifications assumées](#simplifications-assumées)

---

## Architecture des modules

```
.
├── main.c              # point d'entrée, parsing des arguments
├── config.h            # toutes les constantes de jeu
├── core/               # moteur de jeu (sans dépendance SDL ou CLI)
│   ├── structs.h       # types (GameState, City, Unit, ...)
│   ├── core.h          # includes globaux du moteur
│   ├── lifecycle.c/h   # génération de carte, init/libération du jeu
│   ├── cities.c/h      # gestion des villes (rendements, croissance, production)
│   ├── units.c/h       # unités, déplacements, combats
│   ├── turn.c/h        # déroulement d'un tour (phases)
│   ├── tech.c/h        # arbre technologique
│   ├── barbarians.c/h  # IA des barbares
│   ├── save.c/h        # sauvegarde, chargement, scores
│   └── utils.c/h       # RNG, heightmap, graine, distance de Chebyshev
├── cli/
│   ├── cli.c/h         # interface terminal (boucle de jeu + commandes)
│   └── sdl_ui.c/h      # interface graphique SDL2
└── tests/
    └── test_main.c     # tests unitaires
```

Le moteur dans `core/` ne connaît ni SDL ni CLI. Les deux interfaces appellent les mêmes fonctions. Si on voulait ajouter une troisième interface, on n'aurait rien à toucher dans le moteur.

Toutes les constantes numériques du jeu (coûts, bonus, seuils) sont dans `config.h`. Pour changer une valeur, c'est le seul fichier à ouvrir.

---

## Structures de données

Relations entre les structures principales :

```
GameState
├── Map
│    └── Tile[][]
├── Unit*  →  Unit → Unit → Unit → NULL
├── City*  →  City → City → NULL
├── Camp*  →  Camp → Camp → NULL
├── TechState
├── Seed
└── GameResult
```

---

### Tile

```c
typedef struct {
    TerrainType terrain;
    int city_id;   /* -1 = aucune ville */
    int camp_id;   /* -1 = aucun camp */
} Tile;
```

Une case de la carte. On stocke des IDs plutôt que des pointeurs, c'est plus simple à sauvegarder et à comparer. -1 veut dire "rien ici".

---

### Map

```c
typedef struct {
    int width, height;
    Tile **tiles;
} Map;
```

La grille de jeu. `tiles` est un tableau 2D alloué dynamiquement parce qu'on ne connaît les dimensions qu'au lancement. On accède à une case avec `tiles[y][x]`.

On utilise un tableau 2D parce que la taille est fixée une fois pour toutes et que l'accès par `(x, y)` est en O(1). Une liste chaînée n'aurait aucun intérêt ici, on n'ajoute ni ne retire jamais de cases.

---

### TerrainInfo

```c
typedef struct {
    int food, prod, gold, science;
    int move_cost;   /* 0 = infranchissable */
} TerrainInfo;
```

Structure utilitaire qui regroupe les stats d'un type de terrain. Elle n'est pas stockée dans le `GameState`, elle sert uniquement dans les fonctions de calcul de rendement.

---

### BuildingFlag

```c
typedef enum {
    BUILDING_NONE     = 0,
    BUILDING_GRANARY  = (1 << 0),
    BUILDING_WORKSHOP = (1 << 1),
    BUILDING_LIBRARY  = (1 << 2),
    BUILDING_MARKET   = (1 << 3),
    BUILDING_BARRACKS = (1 << 4),
    BUILDING_WALL     = (1 << 5)
} BuildingFlag;
```

Les 6 bâtiments tiennent dans un seul `int` (le champ `buildings` de `City`). Tester si une ville a la Caserne : `city->buildings & BUILDING_BARRACKS`. Plus compact qu'un tableau de booléens, et ça se sauvegarde en un entier.

---

### TechDef

```c
typedef struct {
    int id;
    const char *name;
    int cost;
    int prereqs[4];
    int prereq_count;
    int unlock_warrior, unlock_wall, unlock_library, unlock_market;
    int bonus_food_pct, bonus_prod_pct, bonus_sci_pct, bonus_gold_pct;
    int bonus_pm;
    int bonus_forest_food;
    int bonus_food2_pct;
} TechDef;
```

Définition statique d'une technologie. Utilisé dans un tableau constant de taille 8, pour les 8 technologies, `TechDef TECH_DEFS[8]` dans `tech.c`, jamais modifié en cours de partie. On le sépare de `TechState` pour distinguer ce qui ne change pas (les définitions) de ce qui change (l'état du joueur).

Tableau de taille fixe parce qu'il y a 8 technologies et qu'on y accède par ID en O(1). `prereqs[4]` peut contenir jusqu'à 4 prérequis, les slots inutilisés valent `TECH_ID_NONE`. Commerce en a 2 (Artisanat + Écriture), les autres 0 ou 1.

---

### TechState

```c
typedef struct {
    int acquired[TECH_COUNT];
    int researching;
    int science_stored;
} TechState;
```

État courant de la recherche technologique. `acquired[i]` vaut 1 si la technologie i est acquise. `science_stored` contient les points accumulés vers la recherche en cours ; si on change de tech en cours de route, ce compteur est divisé par 2.

`acquired` est un tableau de taille fixe (8 entrées) parce qu'il y a 8 technologies. Accès O(1) par ID, pas besoin d'autre chose.

---

### Unit

```c
typedef struct Unit {
    int id;
    UnitType type;     /* SETTLER, WARRIOR, BARB */
    UnitOwner owner;   /* PLAYER ou BARB */
    int x, y;
    int hp, hp_max;
    int attack, defense;
    int pm_max, pm_left;
    int maint;
    struct Unit *next;
} Unit;
```

Une unité, qu'elle soit du joueur ou barbare. Toutes les unités sont dans la même liste chaînée, distinguées par `owner`. Ça simplifie les itérations (combat, sauvegarde, comptage) et évite d'avoir à synchroniser deux listes séparées.

On a choisi une liste chaînée plutôt qu'un tableau parce que le nombre d'unités varie constamment (créations, destructions en combat) et qu'on n'accède jamais à une unité par index : on itère sur toutes ou on cherche par ID.

`pm_left` est remis à `pm_max` au début du tour du joueur (ou du tour barbare). `maint` est le coût en or par tour, déduit globalement à chaque fin de tour.

---

### City

```c
typedef struct City {
    int id;
    int x, y;
    int population;
    int food_stock, prod_stock;
    int hp, hp_max;
    int buildings;           /* bitmask BuildingFlag */
    int founded_turn;
    int project_type, project_id;
    int took_damage_this_turn;
    int yield_food, yield_prod, yield_gold, yield_science;  /* cache */
    struct City *next;
} City;
```

Une ville du joueur. Même raisonnement que pour les unités : liste chaînée parce que le nombre de villes varie et qu'on itère dessus sans jamais y accéder par index.

Les quatre champs `yield_*` sont un cache des rendements, recalculés une fois par tour par `cities_compute_yields`. Ça évite de refaire le calcul d'exploitation à chaque accès pendant le tour.

`founded_turn` sert à départager deux villes à égale distance d'une tuile contestée : la plus ancienne l'emporte.

`took_damage_this_turn` empêche la régénération de PV le même tour où la ville a subi une attaque.

---

### Camp

```c
typedef struct Camp {
    int id;
    int x, y;
    int alive;
    struct Camp *next;
} Camp;
```

Un camp barbare. Liste chaînée, même raison que pour les unités et les villes.

Le flag `alive` passe à 0 quand le camp est détruit plutôt que de supprimer le nœud, pour ne pas avoir à mettre à jour les références ailleurs.

---

### Seed

```c
typedef struct {
    uint64_t value;
    char hex[SEED_HEX_LEN + 1];
} Seed;
```

La graine de génération, stockée en deux formats : `uint64_t` pour les calculs RNG, et chaîne hex pour l'affichage et la sauvegarde. Le format hex permet de relancer la même partie depuis un fichier de sauvegarde ou avec `--seed`.

---

### GameResult

```c
typedef enum {
    RESULT_ONGOING = 0,
    RESULT_WIN_SCI,
    RESULT_WIN_TERR,
    RESULT_LOSS_NOCITY,
    RESULT_LOSS_TURNS,
    RESULT_LOSS_PROD
} GameResult;
```

Enum des 5 résultats possibles d'une partie. `GameState.result` le stocke, `game_check_result` le vérifie à chaque fin de tour.

---

### GameState

```c
typedef struct {
    Map map;
    Unit      *units;
    City      *cities;
    Camp      *camps;
    TechState tech;
    int gold;
    int turn, turn_max;
    int n_barbarian_camps_initial, n_camps_destroyed, barb_unit_count;
    int territorial_consecutive_turns, zero_prod_turns;
    int next_unit_id, next_city_id, next_camp_id;
    Seed seed;
    GameResult result;
    int score;
    /* cache des effets technologiques actifs */
    int bonus_food_pct, bonus_prod_pct, bonus_sci_pct, bonus_gold_pct;
    int extra_pm;
    int bonus_forest_food;
    int warrior_unlocked, wall_unlocked, library_unlocked, market_unlocked;
    char player_name[32];
} GameState;
```

L'état global d'une partie. On passe un seul pointeur `GameState *gs` à toutes les fonctions du moteur, ça évite les variables globales.

Le bloc `bonus_*` est un cache des effets technologiques que `tech_apply_effects` recalcule à chaque acquisition. Ça évite de reparcourir toutes les techs acquises à chaque calcul de rendement.

---

### ScoreEntry

```c
typedef struct {
    char name[32];
    char date[32];
    char result[32];
    int score;
    int tour;
} ScoreEntry;
```

Une entrée du tableau des scores, utilisée uniquement pour lire et afficher `scores.txt`. Elle ne fait pas partie du `GameState`.

---

## Algorithmes

### Distance de Chebyshev

La distance de Chebyshev entre deux cases `(x1, y1)` et `(x2, y2)` est `max(|dx|, |dy|)`. Contrairement à la distance euclidienne, elle correspond aux déplacements en 8 directions sur une grille : une case diagonale est à distance 1, pas à distance √2.

On l'utilise partout dans le jeu : rayon d'exploitation des villes, ciblage des barbares, placement de la ville de départ.

---

### Déroulement d'un tour

Chaque tour se déroule dans cet ordre, identique en CLI et en SDL :

1. Phase d'action : le joueur déplace ses unités, donne des ordres de production aux villes, choisit une technologie à rechercher.
2. Phase de production : les rendements sont calculés, les projets de construction et de recherche avancent, l'or et la science sont mis à jour.
3. Phase de croissance : la population de chaque ville augmente ou diminue selon la nourriture disponible.
4. Phase des barbares : spawn des nouvelles unités barbares, puis déplacement et attaques.
5. Fin de tour : régénération des PV des villes, vérification des conditions de victoire et de défaite.

---

### Génération de carte

La carte vient de deux heightmaps indépendantes calculées depuis la graine.

`gen_heightmap` produit un tableau de flottants entre 0 et 1 en superposant 4 couches de bruit. À chaque couche la fréquence double et l'amplitude est divisée par 2, ce qui donne un relief plus naturel que du bruit pur. On génère une table de 256 valeurs aléatoires depuis la seed, puis pour chaque case on calcule un index entre 0 et 255 à partir de ses coordonnées et on lit la valeur dans cette table.

La première heightmap `h` représente l'altitude, la seconde `m` l'humidité. Le terrain de chaque case vient d'un seuillage :

| Condition | Terrain |
|-----------|---------|
| `h < 0.28` | Eau |
| `h < 0.40` et `m > 0.60` | Toundra |
| `h < 0.40` et `m ≤ 0.60` | Désert |
| `h < 0.80` et `m > 0.55` | Forêt |
| `h < 0.80` | Plaine |
| `h ≥ 0.80` | Montagne |

Ça donne des zones cohérentes (continents, côtes, chaînes de montagnes) sans Perlin ni simplex. La graine garantit la reproductibilité.

La ville de départ se pose sur la case Plaine la plus proche du centre, en spirale depuis le centre.

---

### RNG

Le générateur de nombres pseudo-aléatoires est un LCG 64 bits :

```c
uint64_t mix(uint64_t x) {
    return x * 6364136223846793005ULL + 1442695040888963407ULL;
}
```

Les constantes sont celles du LCG de Knuth. On l'applique itérativement pour avancer l'état. Toutes les décisions aléatoires du jeu (placement des camps, heightmap) passent par cette fonction depuis la même graine, ce qui garantit la reproductibilité d'une partie.

La graine peut être fournie avec `--seed`. Si c'est une chaîne quelconque, on la hashe avec FNV-1a pour obtenir le `uint64_t` initial. Si rien n'est fourni :

```c
Seed gen_random_seed(void) {
    Seed s;
    uint64_t x = (uint64_t)time(NULL);
    x ^= (uint64_t)(uintptr_t)&s;
    x ^= (uint64_t)clock();
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    s.value = x;
    compute_hex(&s);
    return s;
}
```

On XOR trois sources (`time`, adresse de `s` sur la pile, `clock`) pour que deux lancements à la même seconde donnent des graines différentes.

---

### Calcul des rendements et exploitation des tuiles

À chaque début de tour, `cities_compute_yields` recalcule les rendements de toutes les villes en deux passes.

Première passe (`cities_accumulate_terrain`) : pour chaque case de la carte, `tile_owner` cherche quelle ville l'exploite. Elle parcourt toutes les villes et retient celle dont la distance de Chebyshev est minimale et dans son rayon. En cas d'égalité, la plus ancienne l'emporte (`founded_turn` le plus petit). Les ressources s'ajoutent au cache de cette ville.

Deuxième passe (`city_apply_bonuses`) : on ajoute les bonus des bâtiments et les pourcentages tech au cache de chaque ville.

Les deux passes garantissent que les conflits de tuiles sont résolus avant d'appliquer les bonus, sans structure intermédiaire.

---

### IA des barbares

Le tour des barbares se déroule en deux étapes : spawn puis déplacement.

Spawn : chaque camp vivant génère une unité barbare si sa case est libre et si le plafond global (`n_camps_initial × 3`) n'est pas atteint.

Déplacement (`barbarians_move_and_attack`) : on copie les IDs des barbares vivants dans un tableau avant de commencer, pour éviter les problèmes de modification de liste pendant l'itération. Pour chaque barbare :

1. `find_nearest_target` cherche la cible la plus proche parmi toutes les unités et villes du joueur (distance de Chebyshev). En cas d'égalité, une unité est préférée à une ville, puis l'ID le plus petit l'emporte. C'est déterministe.

2. Si la cible est à distance 1, le barbare attaque directement.

3. Sinon, `barb_best_step` évalue les 8 cases adjacentes et choisit le pas qui minimise la distance à la cible. En cas d'égalité, on prend le coût le plus faible, puis les coordonnées minimales `(dy, dx)`. Là aussi déterministe.

4. Si la case choisie a une unité ou une ville du joueur, le barbare attaque au lieu de se déplacer.

---

### Combat

Le combat entre deux unités est automatique, résolu en une passe :

```
degats_au_defenseur = max(1, attaque_attaquant − defense_defenseur)
degats_en_retour    = max(0, attaque_defenseur − defense_attaquant)
```

Une unité à 0 PV est détruite immédiatement.

Pour une attaque de ville, la ville utilise une valeur fixe (20, ou 30 avec la Muraille) à la fois en attaque et en défense. Si elle tombe à 0 PV, elle est rasée. Un campement barbare ne se défend pas.

---

### Arbre technologique

L'arbre est un DAG (graphe orienté acyclique) défini statiquement dans `TECH_DEFS`. On vérifie les prérequis dynamiquement : une technologie est proposable si tous ses prérequis ont `acquired[i] == 1`. À chaque acquisition, `tech_apply_effects` remet le cache de `GameState` à zéro et le reconstruit en parcourant toutes les techs acquises.

Changer de recherche en cours de route divise `science_stored` par 2 (arrondi à l'inférieur).

---

## Extension EXT-4

### Menu principal

Au lancement (CLI et SDL), un menu propose trois options : nouvelle partie, charger une sauvegarde, consulter les scores. Les deux interfaces utilisent les mêmes fonctions de sauvegarde.

### Sauvegarde et chargement

Les sauvegardes sont des fichiers texte dans `saves/`, nommés `save_tour<N>_<date>_<heure>.txt`, avec des sections `MAP`, `TECH`, `CITIES`, `UNITS`, `CAMPS`.

La première ligne est toujours `version <N>`. Si la version ne correspond pas à `SAVE_VERSION`, le chargement échoue avec un message d'erreur, sans crash. Un fichier corrompu (lecture incomplète) déclenche un `goto fail` qui libère la mémoire et retourne `NULL`.

### Tableau des scores

À la fin de chaque partie (victoire ou défaite), `scores_append` ajoute une ligne dans `scores.txt` :

```
NomJoueur  2026-05-28  VICTOIRE_SCI  1450  tour:87  seed:0x1111111111111111
```

`scores_read` relit ce fichier et trie les entrées par score décroissant avec `qsort`. Le joueur saisit son nom au lancement d'une nouvelle partie.

---

## Simplifications assumées

- Dissolution non aléatoire : le sujet demande de choisir une unité au hasard, on prend la première de la liste. Le résultat est le même (une unité perdue).

- Règle de départage barbare : en cas de cibles équidistantes, une unité est préférée à une ville, puis l'ID le plus petit l'emporte. Déterministe.

- Warrior de départ : créé gratuitement, sans coût de production.

- Eau infranchissable : l'extension Bateaux (EXT-1) n'est pas implémentée.

- Pas de brouillard de guerre : la carte est entièrement visible. L'extension EXT-6 n'est pas implémentée.
