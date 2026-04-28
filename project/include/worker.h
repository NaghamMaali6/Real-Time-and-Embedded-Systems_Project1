#ifndef WORKER_H
#define WORKER_H

#include "config.h"
#include "pipes.h"
#include "furniture.h" 

void worker_process(int id , Pipes *pipes , Config *cfg , FurnitureSet *furniture , int round , int team_id) ;  //Function prototype for the worker process function that will be called by each worker process to handle the assembly line processing for their assigned pieces based on the configuration, furniture set, and current round of the competition

#endif