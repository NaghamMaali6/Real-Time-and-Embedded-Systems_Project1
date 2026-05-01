#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include "controller.h"
#include "team.h"
#include "furniture.h"
#include "signals.h"

// =========================
// Countdown before start
// =========================
static void countdown()
{
    printf("\nGame starting in...\n") ;
    for (int i = 3 ; i >= 1 ; i--)  //Simple countdown loop to build anticipation before the game starts, printing the countdown numbers and sleeping for 1 second between each number to create a dramatic effect before announcing the start of the game
    {
        printf("%d...\n", i) ;
        sleep(1) ;  //Sleep for 1 second between countdown numbers to create a dramatic effect before announcing the start of the game
    }
    printf("GO!\n\n") ;  //Announce the start of the game after the countdown is complete to signal players to begin their tasks in the assembly line simulation
}

// =====================
// Play a single round
// =====================
static int play_round(Config *cfg , int round , int mode , int min , int max)
{
    printf("\n=============================\n") ;
    printf("       ROUND %d\n" , round) ;
    printf("=============================\n") ;
        
    reset_signals() ;  //Reset completion flags before starting the round

    FurnitureSet set ;  //Declare a FurnitureSet struct to hold the generated pieces for the current round, which will be used by the worker processes in each team to simulate the assembly line processing of furniture pieces in the correct order
    if (init_furniture_set(&set , cfg->num_pieces , mode , min , max) != 0)  //Initialize the furniture set for the current round based on the configuration and generation mode, checking for errors during initialization and exiting if any issues occur to ensure that the game can proceed with a valid set of furniture pieces for the workers to process in the assembly line simulation
    {
        printf("Furniture error\n") ; 
        exit(1) ;  //Exit with error code if furniture initialization fails to ensure proper error handling and prevent undefined behavior in the game due to invalid furniture setup for the workers to process
    }

    countdown() ;  //Call the countdown function to build anticipation before the start of the round, allowing players to prepare for the upcoming round and creating a dramatic effect before announcing the start of the round

    fflush(stdout) ;  //This prevents log messages from being duplicated or printed out of order across processes
    pid_t t0 = fork() ;  //Fork a new process for Team 0 to enable concurrent execution of the worker processes for Team 0, allowing the controller to manage both teams simultaneously and determine the winner of the round based on the signals received from the worker processes in each team
    if (t0 < 0)  //Check for errors during fork and exit if any issues occur to ensure proper error handling and prevent undefined behavior in the game due to failure in creating a new process for Team 0, which is essential for the concurrent execution of the worker processes and the overall flow of the game
    { 
        perror("fork") ; 
        exit(1) ; 
    }

    if (t0 == 0)
    {
        setpgid(0 , 0) ;  //CREATE PROCESS GROUP FOR TEAM 0
            
        start_team(cfg , &set , round , 0) ;  //Call the start_team function for Team 0 to create the worker processes and set up the pipes for inter-process communication based on the provided configuration, furniture set, current round, and team ID, allowing Team 0 to run its assembly line simulation concurrently with Team 1 and compete to complete the sequence of furniture pieces in the correct order before Team 1
        
        kill(getppid() , SIGUSR1) ;  //Send a signal to the parent process (controller) to notify that Team 0 has completed its tasks, allowing the controller to monitor the progress of Team 0 and determine the winner of the round based on the signals received from both teams(STOP Team 1)
        
        exit(0) ;
    }

    fflush(stdout) ;  //This prevents log messages from being duplicated or printed out of order across processes
    pid_t t1 = fork() ;  //Fork a new process for Team 1 to enable concurrent execution of the worker processes for Team 1, allowing the controller to manage both teams simultaneously and determine the winner of the round based on the signals received from the worker processes in each team
    if (t1 < 0)  //Check for errors during fork and exit if any issues occur to ensure proper error handling and prevent undefined behavior in the game due to failure in creating a new process for Team 1, which is essential for the concurrent execution of the worker processes and the overall flow of the game
    { 
        perror("fork") ; 
        exit(1) ; 
    }
    if (t1 == 0)
    {
        setpgid(0 , 0) ;  //CREATE PROCESS GROUP FOR TEAM 1
            
        start_team(cfg , &set , round , 1) ;  //Call the start_team function for Team 1 to create the worker processes and set up the pipes for inter-process communication based on the provided configuration, furniture set, current round, and team ID, allowing Team 1 to run its assembly line simulation concurrently with Team 0 and compete to complete the sequence of furniture pieces in the correct order before Team 0
            
        kill(getppid() , SIGUSR2) ;  //Send a signal to the parent process (controller) to notify that Team 1 has completed its tasks, allowing the controller to monitor the progress of Team 1 and determine the winner of the round based on the signals received from both teams(STOP Team 0)
            
        exit(0) ;
    }

    sigset_t mask, oldmask ;
    //mask: set of signals we want to block
    //oldmask: stores previous signal mask to restore later

    sigemptyset(&mask) ;
    sigaddset(&mask , SIGUSR1) ;  //Initialize mask to empty (no signals blocked initially)
    sigaddset(&mask , SIGUSR2) ;  //Add SIGUSR1 and SIGUSR2 to the mask(these signals indicate that Team 0 or Team 1 has finished)
    sigprocmask(SIG_BLOCK , &mask , &oldmask) ;  //Block SIGUSR1 and SIGUSR2 temporarily (preventing race conditions where signals arrive before we are ready to wait for them)

    while (!team0_done && !team1_done)  //Wait until either Team 0 or Team 1 finishes
        sigsuspend(&oldmask) ;  //temporarily replace the signal mask with oldmask (unblocking SIGUSR1/SIGUSR2 and puting the process to sleep atomically until a signal is received)

    sigprocmask(SIG_UNBLOCK , &mask , NULL) ;  //Unblock SIGUSR1 and SIGUSR2 after finishing the wait

    int winner ;

    if (team0_done)
    {
        printf("Team 0 wins ROUND %d\n" , round) ;  //Print the winner of the round based on which team sent the signal first, allowing the controller to announce the outcome of the round to the players and update the scores accordingly
                        
        kill(-t1 , SIGTERM) ;  //STOP Team 1

        winner = 0 ;
    }
    else
    {
        printf("Team 1 wins ROUND %d\n" , round) ;  //Print the winner of the round based on which team sent the signal first, allowing the controller to announce the outcome of the round to the players and update the scores accordingly
                        
        kill(-t0 , SIGTERM) ;  //STOP Team 0

        winner = 1 ;
    }

    waitpid(t0 , NULL , 0) ;  //Wait for the Team 0 process to finish to ensure that the controller does not exit prematurely and allows all worker processes in Team 0 to complete their tasks before proceeding with cleanup and termination of the team
    waitpid(t1 , NULL , 0) ;  //Wait for the Team 1 process to finish to ensure that the controller does not exit prematurely and allows all worker processes in Team 1 to complete their tasks before proceeding with cleanup and termination of the team

    free_furniture_set(&set) ;  //Free the dynamically allocated memory in the furniture set after the round is complete to prevent memory leaks and ensure proper cleanup of resources used for the assembly line simulation in the current round before proceeding to the next round or extra rounds in case of a draw

    return winner;
}

// =====================
// Run full controller
// =====================
void run_controller(Config *cfg , int mode , int min , int max)
{
    setup_signals() ;  //Setup signal handlers to allow the controller to receive notifications from worker processes when they complete their tasks, enabling the controller to monitor the progress of each team and determine the winner of each round based on the signals received from the workers

    int score0 = 0 ;  //Initialize score for Team 0 to keep track of the number of rounds won by Team 0 throughout the game, allowing the controller to determine the overall winner at the end of all rounds (including extra rounds in case of a draw) and announce the final result to the players
    int score1 = 0 ;  //Initialize score for Team 1 to keep track of the number of rounds won by Team 1 throughout the game, allowing the controller to determine the overall winner at the end of all rounds (including extra rounds in case of a draw) and announce the final result to the players

    int round = 1 ;  //Initialize round counter to keep track of the current round number being played, allowing the controller to manage the flow of the game and determine when to start extra rounds in case of a draw after the initial rounds have been completed
    int target = cfg->rounds_to_win ;  //Set target number of rounds to win based on the configuration, which determines how many rounds a team needs to win in order to be declared the overall winner of the game, and allows the controller to manage the flow of the game accordingly

    // ===============================
    // 1) PLAY FIXED NUMBER OF ROUNDS
    // ===============================
    for (round = 1; round <= target; round++)
    {
        int winner = play_round(cfg , round , mode , min , max) ;
        if (winner == 0)
        {            
            score0++ ;  //Increment the score for Team 0 if they won the round, allowing the controller to keep track of the number of rounds won by Team 0 throughout the game and determine the overall winner at the end of all rounds (including extra rounds in case of a draw) based on the final scores
        }
        else
        {            
            score1++ ;  //Increment the score for Team 1 if they won the round, allowing the controller to keep track of the number of rounds won by Team 1 throughout the game and determine the overall winner at the end of all rounds (including extra rounds in case of a draw) based on the final scores            
        }
    }

    // =====================================
    // 2) EXTRA ROUNDS IF DRAW(tie-breaker)
    // =====================================
    while (score0 == score1)  //Check for a draw after the initial rounds have been completed, and if there is a draw, enter a loop to play extra rounds until one team wins to break the tie and determine the overall winner of the game, allowing for a fair and competitive resolution in case both teams have the same score after the initial rounds
    {
        printf("\nDraw (%d - %d)\nExtra round!\n" , score0 , score1) ;  //Print a message indicating that there is a draw and an extra round will be played to break the tie, allowing the controller to inform the players about the current status of the game and the need for an extra round to determine the winner

        int winner = play_round(cfg , round , mode , min , max) ;

        if (winner == 0)
            score0++ ;
        else
            score1++ ;

        round++ ;  //Increment the round counter for the next round or extra round in case of a draw, allowing the controller to keep track of the current round number being played and manage the flow of the game accordingly
    }

    // =============
    // FINAL RESULT
    // =============
    printf("\n=============================\n") ;
    printf("        FINAL RESULT\n");
    printf("=============================\n") ;

    printf("Team 0 score: %d\n" , score0) ;
    printf("Team 1 score: %d\n" , score1) ;

    if (score0 > score1)  //Check final scores to determine the overall winner of the game after all rounds (including extra rounds in case of a draw) have been completed, and print the final result to announce the winning team
        printf("TEAM 0 WINS THE GAME!\n") ;
    else
        printf("TEAM 1 WINS THE GAME!\n") ;

    printf("Congratulations!!!\nthank you all for playing!\n") ;  
}

