#include "signals.h"
#include <stdio.h>

// ================================
// Global flags (shared with main)
// ================================
volatile sig_atomic_t team0_done = 0 ;  //Flag indicating if first Team has completed 
volatile sig_atomic_t team1_done = 0 ;  //Flag indicating if second Team has completed 

// =========
// Handlers
// =========
static void handle_team0(int sig)
{
    (void)sig ;  //Unused parameter to avoid compiler warning about unused variable
    team0_done = 1 ;  //Set the flag to indicate Team 0 has completed when the signal is received
}

static void handle_team1(int sig)
{
    (void)sig ;  //Unused parameter to avoid compiler warning about unused variable
    team1_done = 1 ;  //Set the flag to indicate Team 1 has completed when the signal is received
}

// ===============
// Setup signals
// ===============
void setup_signals()
{
    struct sigaction sa0 , sa1 ;  //Declare sigaction structures for Team 0 and Team 1 signal handlers

    sa0.sa_handler = handle_team0 ;  //Set the handler function for Team 0 signal
    sigemptyset(&sa0.sa_mask) ;  //Initialize the signal mask to be empty (no signals blocked during handler execution)
    sa0.sa_flags = SA_RESTART ;  //Automatically restart interrupted system calls (e.g., pause, waitpid) after signal handling to Ensure stable execution by preventing syscalls from failing due to signal interruptions

    sa1.sa_handler = handle_team1 ;  //Set the handler function for Team 1 signal
    sigemptyset(&sa1.sa_mask) ;  //Initialize the signal mask to be empty (no signals blocked during handler execution)
    sa1.sa_flags = SA_RESTART ;  //Automatically restart interrupted system calls (e.g., pause, waitpid) after signal handling to Ensures stable execution by preventing syscalls from failing due to signal interruptions

    sigaction(SIGUSR1 , &sa0 , NULL) ;  //Register the handler for SIGUSR1 to handle Team 0 completion notifications
    sigaction(SIGUSR2 , &sa1 , NULL) ;  //Register the handler for SIGUSR2 to handle Team 1 completion notifications
}

// ==============================
// Reset flags before new round
// ==============================
void reset_signals()
{
    team0_done = 0 ;  //Reset Team 0 completion flag before starting a new round to ensure accurate tracking of team completion status for the upcoming round
    team1_done = 0 ;  //Reset Team 1 completion flag before starting a new round to ensure accurate tracking of team completion status for the upcoming round
}