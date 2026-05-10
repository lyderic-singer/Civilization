#include "tech.h"

static const TechDef TECH_DEFS[TECH_COUNT] = {
    /* TECH_ID_HUNTING = 0 */
    {
        .id = TECH_ID_HUNTING,
        .name = "Chasse",
        .cost = TECH_COST_HUNTING,
        .prereqs = {TECH_ID_NONE, TECH_ID_NONE, TECH_ID_NONE, TECH_ID_NONE},
        .prereq_count = 0,
        .bonus_forest_food = 1,
    },
    /* TECH_ID_FARMING = 1 */
    {
        .id = TECH_ID_FARMING,
        .name = "Agriculture",
        .cost = TECH_COST_FARMING,
        .prereqs = {TECH_ID_NONE, TECH_ID_NONE, TECH_ID_NONE, TECH_ID_NONE},
        .prereq_count = 0,
        .bonus_food_pct = 10,
    },
    /* TECH_ID_IRRIGATION = 2 */
    {
        .id = TECH_ID_IRRIGATION,
        .name = "Irrigation",
        .cost = TECH_COST_IRRIGATION,
        .prereqs = {TECH_ID_FARMING, TECH_ID_NONE, TECH_ID_NONE, TECH_ID_NONE},
        .prereq_count = 1,
        .bonus_food2_pct = 20,
    },
    /* TECH_ID_CRAFT = 3 */
    {
        .id = TECH_ID_CRAFT,
        .name = "Artisanat",
        .cost = TECH_COST_CRAFT,
        .prereqs = {TECH_ID_NONE, TECH_ID_NONE, TECH_ID_NONE, TECH_ID_NONE},
        .prereq_count = 0,
        .unlock_warrior = 1,
        .bonus_prod_pct = 10,
    },
    /* TECH_ID_MASONRY = 4 */
    {
        .id = TECH_ID_MASONRY,
        .name = "Maconnerie",
        .cost = TECH_COST_MASONRY,
        .prereqs = {TECH_ID_CRAFT, TECH_ID_NONE, TECH_ID_NONE, TECH_ID_NONE},
        .prereq_count = 1,
        .unlock_wall = 1,
    },
    /* TECH_ID_WRITING = 5 */
    {
        .id = TECH_ID_WRITING,
        .name = "Ecriture",
        .cost = TECH_COST_WRITING,
        .prereqs = {TECH_ID_NONE, TECH_ID_NONE, TECH_ID_NONE, TECH_ID_NONE},
        .prereq_count = 0,
        .unlock_library = 1,
        .bonus_sci_pct = 10,
    },
    /* TECH_ID_COMMERCE = 6 */
    {
        .id = TECH_ID_COMMERCE,
        .name = "Commerce",
        .cost = TECH_COST_COMMERCE,
        .prereqs = {TECH_ID_CRAFT, TECH_ID_WRITING, TECH_ID_NONE, TECH_ID_NONE},
        .prereq_count = 2,
        .unlock_market = 1,
        .bonus_gold_pct = 10,
    },
    /* TECH_ID_RIDING = 7 */
    {
        .id = TECH_ID_RIDING,
        .name = "Equitation",
        .cost = TECH_COST_RIDING,
        .prereqs = {TECH_ID_NONE, TECH_ID_NONE, TECH_ID_NONE, TECH_ID_NONE},
        .prereq_count = 0,
        .bonus_pm = 1,
    },
};

/* ═══════════════════════════════════════════════════════════
   PUBLIC API
═══════════════════════════════════════════════════════════ */

const TechDef *tech_def(int tech_id) {
    if (tech_id < 0 || tech_id >= TECH_COUNT) return NULL;
    return &TECH_DEFS[tech_id];
}

const char *tech_name(int tech_id) {
    if (tech_id < 0 || tech_id >= TECH_COUNT) return "Inconnue";
    return TECH_DEFS[tech_id].name;
}

void tech_init(TechState *ts) {
    memset(ts->acquired, 0, sizeof ts->acquired);
    ts->researching    = TECH_ID_NONE;
    ts->science_stored = 0;
}

int tech_prereqs_met(const TechState *ts, int tech_id) {
    if (tech_id < 0 || tech_id >= TECH_COUNT) return 0;
    const TechDef *def = &TECH_DEFS[tech_id];
    for (int i = 0; i < def->prereq_count; i++) {
        int p = def->prereqs[i];
        if (p == TECH_ID_NONE) continue;
        if (!ts->acquired[p]) return 0;
    }
    return 1;
}

int tech_available(const TechState *ts, int tech_id) {
    if (tech_id < 0 || tech_id >= TECH_COUNT) return 0;
    if (ts->acquired[tech_id]) return 0;
    return tech_prereqs_met(ts, tech_id);
}

void tech_set_research(TechState *ts, int tech_id) {
    if (ts->researching != TECH_ID_NONE && ts->researching != tech_id) {
        /* Switching: lose half accumulated science */
        ts->science_stored /= 2;
        printf("[Recherche] Changement vers %s, cagnotte reduite a %d\n",
               tech_name(tech_id), ts->science_stored);
    }
    ts->researching = tech_id;
}

int tech_add_science(TechState *ts, int science) {
    if (ts->researching == TECH_ID_NONE) return TECH_ID_NONE;
    ts->science_stored += science;
    const TechDef *def = &TECH_DEFS[ts->researching];
    if (ts->science_stored >= def->cost) {
        int completed = ts->researching;
        ts->acquired[completed]  = 1;
        ts->researching          = TECH_ID_NONE;
        ts->science_stored       = 0;
        printf("[Recherche] Technologie acquise: %s\n", def->name);
        return completed;
    }
    return TECH_ID_NONE;
}

int tech_all_acquired(const TechState *ts) {
    for (int i = 0; i < TECH_COUNT; i++)
        if (!ts->acquired[i]) return 0;
    return 1;
}

/* ═══════════════════════════════════════════════════════════
   EFFECT APPLICATION
   Recomputes all bonus caches from scratch each call.
   This avoids cumulative drift when called multiple times.
═══════════════════════════════════════════════════════════ */

void tech_apply_effects(GameState *gs) {
    /* Reset all bonus caches */
    gs->bonus_food_pct     = 0;
    gs->bonus_prod_pct     = 0;
    gs->bonus_sci_pct      = 0;
    gs->bonus_gold_pct     = 0;
    gs->extra_pm           = 0;
    gs->bonus_forest_food  = 0;
    gs->warrior_unlocked   = 0;
    gs->wall_unlocked      = 0;
    gs->library_unlocked   = 0;
    gs->market_unlocked    = 0;

    for (int i = 0; i < TECH_COUNT; i++) {
        if (!gs->tech.acquired[i]) continue;
        const TechDef *d = &TECH_DEFS[i];
        gs->bonus_food_pct    += d->bonus_food_pct + d->bonus_food2_pct;
        gs->bonus_prod_pct    += d->bonus_prod_pct;
        gs->bonus_sci_pct     += d->bonus_sci_pct;
        gs->bonus_gold_pct    += d->bonus_gold_pct;
        gs->extra_pm          += d->bonus_pm;
        gs->bonus_forest_food += d->bonus_forest_food;
        if (d->unlock_warrior) gs->warrior_unlocked = 1;
        if (d->unlock_wall)    gs->wall_unlocked    = 1;
        if (d->unlock_library) gs->library_unlocked = 1;
        if (d->unlock_market)  gs->market_unlocked  = 1;
    }

    /* Propagate new PM to all existing player units */
    units_apply_pm_bonus(gs);
}