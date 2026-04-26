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
// Run full controller
// =====================
void run_controller(Config *cfg , int mode , int min , int max)
{
    setup_signals() ;  //Setup signal handlers to allow the controller to respond to team completion notifications and manage the flow of the game based on signals received from the worker processes in each team, enabling synchronization and coordination between the controller and the teams during the game rounds

    int score0 = 0 ;
    int score1 = 0 ;

    int round = 1 ;

    int target = cfg->rounds_to_win ;

    while (1) 
    {
        printf("\n=============================\n") ;
        printf("       ROUND %d\n" , round) ;
        printf("=============================\n") ;

        reset_signals() ;  //Reset signal flags before starting a new round to ensure accurate tracking of team completion status for the upcoming round, allowing the controller to correctly determine the winner of each round based on the signals received from the worker processes in each team during that round

        //Prepare SAME furniture for fairness:
        FurnitureSet set ;
        if (init_furniture_set(&set , cfg->num_pieces , mode , min , max) != 0)  //Check for errors during furniture set initialization and exit if any issues occur to ensure proper error handling and prevent undefined behavior in the game simulation
        {
            printf("Furniture error\n") ;
            exit(1) ;  //Exit with error code if furniture initialization fails to ensure proper error handling and prevent undefined behavior in the game simulation
        }

        countdown() ;  //Start countdown before the round starts to build anticipation and signal players to prepare for the upcoming round in the assembly line simulation

        // =============
        // Fork TEAM 0
        // =============
        pid_t t0 = fork() ;  //Fork a new process for Team 0 to enable concurrent execution of both teams in the assembly line simulation, allowing them to compete against each other in the same round while the controller manages the flow of the game and determines the winner based on signals received from the worker processes in each team
        
        if (t0 < 0)  //Check for errors during fork and exit if any issues occur to ensure proper error handling and prevent undefined behavior in the game simulation
        {
            perror("fork failed") ;
            exit(1) ;
        }
        
        if (t0 == 0)
        {
            start_team(cfg , &set , round , 0) ;  //Call the start_team function for Team 0 with the appropriate parameters to initialize and run the team, allowing it to execute its worker processes and compete in the assembly line simulation for the current round
            kill(getppid() , SIGUSR1) ;  //notify controller of completion by sending SIGUSR1 signal to the parent process (controller) to indicate that Team 0 has completed its tasks for the current round, allowing the controller to track team completion status and determine the winner of the round based on the signals received from both teams
            exit(0) ; 
        }

        // =============
        // Fork TEAM 1
        // =============
        pid_t t1 = fork() ;  //Fork a new process for Team 1 to enable concurrent execution of both teams in the assembly line simulation, allowing them to compete against each other in the same round while the controller manages the flow of the game and determines the winner based on signals received from the worker processes in each team
        
        if (t1 < 0)  //Check for errors during fork and exit if any issues occur to ensure proper error handling and prevent undefined behavior in the game simulation
        {
            perror("fork failed") ;
            exit(1) ;
        }
        
        if (t1 == 0)
        {
            start_team(cfg , &set , round , 1) ;  //Call the start_team function for Team 1 with the appropriate parameters to initialize and run the team, allowing it to execute its worker processes and compete in the assembly line simulation for the current round
            kill(getppid() , SIGUSR2) ;  //notify controller of completion by sending SIGUSR2 signal to the parent process (controller) to indicate that Team 1 has completed its tasks for the current round, allowing the controller to track team completion status and determine the winner of the round based on the signals received from both teams
            exit(0) ;
        }

        // ================
        // Wait for winner
        // ================
        while (!team0_done && !team1_done)  //Wait in a loop until either team signals completion by setting their respective flags, allowing the controller to determine the winner of the round based on which team signals completion first, creating a competitive and engaging gaming experience in the assembly line simulation
            pause() ;  //wait for signal

        // ==============
        // Decide winner
        // ==============
        if (team0_done)
        {
            printf("Team 0 wins ROUND %d\n" , round) ;
            score0++ ;  //Increment Team 0's score if it wins the round to keep track of the team's performance and determine the overall winner of the game based on the scores accumulated across all rounds, allowing for a competitive and engaging gaming experience in the assembly line simulation
        }
        else
        {
            printf("Team 1 wins ROUND %d\n" , round) ;  
            score1++ ;  //Increment Team 1's score if it wins the round to keep track of the team's performance and determine the overall winner of the game based on the scores accumulated across all rounds, allowing for a competitive and engaging gaming experience in the assembly line simulation
        }

        //cleanup children:
        waitpid(t0 , NULL , 0) ;  //Wait for Team 0's process to finish to ensure that all worker processes in Team 0 have completed their tasks and the team has fully finished before proceeding with cleanup and starting the next round, allowing for proper synchronization and resource management in the game simulation
        waitpid(t1 , NULL , 0) ;  //Wait for Team 1's process to finish to ensure that all worker processes in Team 1 have completed their tasks and the team has fully finished before proceeding with cleanup and starting the next round, allowing for proper synchronization and resource management in the game simulation

        free_furniture_set(&set) ;  //Free the furniture set after the round is complete to clean up allocated resources and prevent memory leaks, ensuring proper resource management in the game simulation as the controller prepares for the next round with a new furniture set for both teams to compete with
        
        // =========================
        // Check win condition
        // =========================
        if (score0 == target || score1 == target)
            break;  //Exit loop if either team has reached the target score to determine the overall winner of the game and end the simulation, allowing for a competitive and engaging gaming experience in the assembly line simulation

        // ====================
        // Extra round if draw
        // ====================
        if (score0 == score1)
        {
            printf("\nDraw (%d - %d) \nExtra round!\n", score0 , score1) ;  //If the scores are tied after a round, print a message indicating a draw and that an extra round will be played to break the tie, adding excitement and unpredictability to the game simulation as both teams compete for the win in the next round
        }
        
        round++ ;  //Increment round number for the next round of the game to keep track of the current round and provide context for the players and the controller as they progress through the game simulation, allowing for a structured and engaging gaming experience in the assembly line simulation
    }

    // =============
    // Final Winner
    // =============
    printf("\n=============================\n") ;
    printf("        FINAL RESULT\n");
    printf("=============================\n") ;

    printf("Team 0 score: %d\n" , score0) ;
    printf("Team 1 score: %d\n" , score1) ;

    if (score0 > score1)  //If Team 0 has a higher score than Team 1, declare Team 0 as the winner and print a congratulatory message to celebrate their victory in the game simulation, providing closure and recognition for their performance throughout the rounds of the assembly line simulation
        printf("TEAM 0 WINS THE GAME!\n") ;
    else
        printf("TEAM 1 WINS THE GAME!\n") ;
}