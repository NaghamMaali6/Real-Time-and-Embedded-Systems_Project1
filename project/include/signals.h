#ifndef SIGNALS_H
#define SIGNALS_H

#include <signal.h>

// ================================
// Flags updated by signal handlers
// ================================
extern volatile sig_atomic_t teamA_done ;  //Flag indicating if Team A has completed
extern volatile sig_atomic_t teamB_done ;  //Flag indicating if Team B has completed

void setup_signals() ;  //Setup all signal handlers

void reset_signals() ;  //Reset flags before each round

#endif