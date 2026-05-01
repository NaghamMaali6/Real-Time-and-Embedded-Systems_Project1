#ifndef SIGNALS_H
#define SIGNALS_H

#include <signal.h>

// ================================
// Flags updated by signal handlers
// ================================
extern volatile sig_atomic_t team0_done ;  //Flag indicating if first Team has completed
extern volatile sig_atomic_t team1_done ;  //Flag indicating if second Team has completed
//extern = ONE shared instance

void setup_signals() ;  //Setup all signal handlers

void reset_signals() ;  //Reset flags before each round

#endif