#ifndef TECH_H
#define TECH_H

#include "core.h"

/* Get the static definition of a technology */
const TechDef *tech_def(int tech_id);

/* Initialize TechState (all unacquired, no research) */
void tech_init(TechState *ts);

/* Returns 1 if all prerequisite techs for tech_id are acquired */
int tech_prereqs_met(const TechState *ts, int tech_id);

/* Returns 1 if the tech can currently be researched
   (not acquired, all prereqs met) */
int tech_available(const TechState *ts, int tech_id);

/* Set the current research target.
   If switching mid-research, halves the stored science (floor). */
void tech_set_research(TechState *ts, int tech_id);

/* Add science to the current research pool.
   Returns the tech_id if a tech was completed, TECH_ID_NONE otherwise.
   Resets science_stored to 0 on completion (surplus is lost). */
int tech_add_science(TechState *ts, int science);

/* Apply all acquired tech effects to GameState caches.
   Call after any tech is acquired. */
void tech_apply_effects(GameState *gs);

/* Returns 1 if all techs are acquired (scientific victory condition) */
int tech_all_acquired(const TechState *ts);

/* Human-readable name */
const char *tech_name(int tech_id);

#endif /* TECH_H */