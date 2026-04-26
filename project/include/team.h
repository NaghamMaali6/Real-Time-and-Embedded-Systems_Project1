#ifndef TEAM_H
#define TEAM_H

#include "config.h"
#include "furniture.h" 

// Start one team (one pipeline of workers)
void start_team(Config *cfg, FurnitureSet *furniture, int round);

#endif