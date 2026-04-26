#ifndef WORKER_H
#define WORKER_H

#include "config.h"
#include "pipes.h"
#include "furniture.h" 

// Main worker entry
void worker_process(int id, Pipes *pipes, Config *cfg, FurnitureSet *furniture, int round);

#endif