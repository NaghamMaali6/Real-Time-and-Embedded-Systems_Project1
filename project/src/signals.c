#include <signal.h>
#include <stdio.h>

volatile int winner_flag = 0 ;  //Global variable to indicate which team won (0 = none, 1 = team 1, 2 = team 2)

//Handler for signals:
void handle_signal(int sig)
{
    if (sig == SIGUSR1)
    {
        winner_flag = 1 ; //Team 1 won
    }
    else if (sig == SIGUSR2)
    {
        winner_flag = 2 ; //Team 2 won
    }
}

//Setup signal handlers:
void setup_signals()
{
    signal(SIGUSR1 , handle_signal) ;
    signal(SIGUSR2 , handle_signal) ;
}