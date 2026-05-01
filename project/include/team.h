#ifndef TEAM_H
#define TEAM_H

#include "config.h"
#include "furniture.h" 

void start_team(Config *cfg , FurnitureSet *furniture , int round , int team_id) ;  //Function prototype for starting a team, which involves creating worker processes and setting up pipes for inter-process communication based on the provided configuration, furniture set, current round, and team ID to run the assembly line simulation for that team and round

#endif