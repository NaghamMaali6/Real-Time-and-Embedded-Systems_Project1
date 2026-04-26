#include "signals.h"
#include <stdio.h>

// ================================
// Global flags (shared with main)
// ================================
volatile sig_atomic_t teamA_done = 0 ;  //Flag indicating if Team A has completed 
volatile sig_atomic_t teamB_done = 0 ;  //Flag indicating if Team B has completed 

// =========
// Handlers
// =========
static void handle_teamA(int sig)
{
    (void)sig ;  //Unused parameter to avoid compiler warning about unused variable
    teamA_done = 1 ;  //Set the flag to indicate Team A has completed when the signal is received
}

static void handle_teamB(int sig)
{
    (void)sig ;  //Unused parameter to avoid compiler warning about unused variable
    teamB_done = 1 ;  //Set the flag to indicate Team B has completed when the signal is received
}

// ===============
// Setup signals
// ===============
void setup_signals()
{
    struct sigaction saA ;
    struct sigaction saB ;

    saA.sa_handler = handle_teamA ;  //Set the handler function for Team A completion signal
    sigemptyset(&saA.sa_mask) ;  //Initialize the signal mask to empty for Team A handler to allow all signals while handling
    saA.sa_flags = 0 ;  //No special flags for Team A handler

    saB.sa_handler = handle_teamB ;  //Set the handler function for Team B completion signal
    sigemptyset(&saB.sa_mask) ;  //Initialize the signal mask to empty for Team B handler to allow all signals while handling
    saB.sa_flags = 0 ;  //No special flags for Team B handler

    sigaction(SIGUSR1 , &saA , NULL) ;  //Team A completion signal handler setup to handle SIGUSR1 signals sent by workers when Team A completes their tasks
    sigaction(SIGUSR2 , &saB , NULL) ;  //Team B completion signal handler setup to handle SIGUSR2 signals sent by workers when Team B completes their tasks
}

// ==============================
// Reset flags before new round
// ==============================
void reset_signals()
{
    teamA_done = 0 ;  //Reset the flag for Team A completion to 0 before starting a new round to ensure that the main process can accurately track the completion status of Team A for each round without interference from previous rounds
    teamB_done = 0 ;  //Reset the flag for Team B completion to 0 before starting a new round to ensure that the main process can accurately track the completion status of Team B for each round without interference from previous rounds
}