#ifndef SIGNALS_H
#define SIGNALS_H

void setup_signals() ;  //Setup signal handlers for winner notification
extern volatile int winner_flag ;  //Global variable to indicate which team won (0 = none, 1 = team 1, 2 = team 2)

#endif