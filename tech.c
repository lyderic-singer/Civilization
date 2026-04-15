#include "tech.h"
#include "units.h"
#include "config.h"

#include <stdio.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════
   TECH DEFINITIONS
   Graph (DAG):
     Hunting    <- start
     Farming    <- start
     Irrigation <- Farming
     Craft      <- start
     Masonry    <- Craft
     Writing    <- start
     Commerce   <- Craft + Writing
     Riding     <- start
═══════════════════════════════════════════════════════════ */

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
