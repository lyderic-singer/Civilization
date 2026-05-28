#ifndef TECH_H
#define TECH_H

#include "structs.h"

const TechDef *tech_def(int tech_id);
void tech_init(TechState *ts);
int tech_available(const TechState *ts, int tech_id);
void tech_set_research(TechState *ts, int tech_id);
int tech_add_science(TechState *ts, int science);
void tech_apply_effects(GameState *gs);
int tech_all_acquired(const TechState *ts);
const char *tech_name(int tech_id);

#endif
