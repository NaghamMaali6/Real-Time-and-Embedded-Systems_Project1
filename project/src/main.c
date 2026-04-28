#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <unistd.h>
#include <sys/wait.h>
#include "config.h"  
#include "furniture.h"
#include "pipes.h"
#include "team.h"
#include "controller.h"
#include "signals.h"

// ==============================================================
// Function: try_open
// Purpose : Attempt to open a file from multiple possible paths
// ==============================================================
FILE* try_open(const char *filename , char *resolved_path)
{
    FILE *f ;  //File pointer used to attempt opening the file

    // ------------------------------------------------------
    // 1. Try opening the file exactly as entered by the user
    // ------------------------------------------------------
    f = fopen(filename , "r") ;   //Try opening file in read mode

    if (f)   //If fopen succeeded (file exists and is accessible)
    {
        strcpy(resolved_path , filename) ;  //Copy the valid path into resolved_path
        return f ;  //Return the opened file pointer
    }

    // ------------------------------------------------------
    // 2. Try opening file inside "project/config/" directory
    // ------------------------------------------------------
    snprintf(resolved_path , 512 , "project/config/%s" , filename) ;  //Build a new path safely using snprintf to avoid buffer overflow

    f = fopen(resolved_path , "r") ;  //Try opening using the new path

    if (f) return f ;  //If successful, return file pointer immediately

    // ------------------------------------------------------
    // 3. Try opening file inside "config/" directory
    // ------------------------------------------------------
    snprintf(resolved_path , 512 , "config/%s" , filename) ;  //Construct alternative path

    f = fopen(resolved_path , "r") ;  //Try opening file again

    if (f) return f ;  //If successful, return file pointer

    // ------------------------------------------------------
    // If all attempts fail, file not found
    // ------------------------------------------------------
    return NULL;  //Return NULL to indicate failure
}

// ==========================================================
// Function: main
// Purpose : Entry point of the program
// ==========================================================
int main()
{
    char filename[256] ;  //Buffer to store user input file name
    char fullpath[512] ;  //Buffer to store resolved full path of file
    int choice ;  //Variable to store user choice (exit or retry)
    Config cfg ;  //Config struct to hold loaded configuration values

    // ------------------------
    // Display welcome message
    // ------------------------
    printf("==========================================\n") ;
    printf(" Hi! Welcome to Furnishing Simulator Game \n") ;
    printf("==========================================\n") ;

    printf("Two teams compete to pass furniture pieces in the correct order; the first to complete the sequence wins each round, and the first to reach the target score wins the game (extra rounds break ties).\n") ;  //Brief game description to inform the user about the objective and rules of the game, setting expectations and providing context for the gameplay experience 

    // ---------------------------------------------------------
    // Infinite loop until valid file is provided or user exits
    // ---------------------------------------------------------
    while (1)
    {
        printf("\nEnter config file name: ") ;  //Ask user to enter config file name

        scanf("%255s" , filename) ;  //Read input safely (max 255 characters to avoid overflow)

        FILE *test = try_open(filename , fullpath) ;  //Try to open file using helper function

        // --------------------------------------------------
        // Case 1: File NOT found
        // --------------------------------------------------
        if (test == NULL)
        {
            printf("\nError: Cannot open file '%s'\n" , filename) ;

            printf("1 - Exit\n2 - Try again\nChoice: ") ;  //Ask user what to do next

            scanf("%d" , &choice) ;  //Read user decision

            if (choice == 1)
            {
                //If User chose to exit program(1):
                printf("\nGoodbye!\n") ;
                return 0 ;  //Terminate program successfully
            }
            else
            {
                continue ;  //User chose to retry and loop continues
            }
        }

        // --------------------------------------------------
        // Case 2: File found successfully
        // --------------------------------------------------
        fclose(test) ;  //Close test file (we only checked existence)

        printf("\nUsing config file: %s\n" , fullpath) ;

        cfg = load_config(fullpath) ;  //Load and validate configuration using config.c logic

        // --------------------------------------------------
        // Display loaded configuration values (test output)
        // --------------------------------------------------
        printf("\nConfiguration Loaded Successfully!\n") ;
        printf("-------------------------------------\n") ;

        printf("Workers: %d\n" , cfg.num_workers) ;  //Print number of workers
        printf("Pieces: %d\n" , cfg.num_pieces) ;  //Print number of pieces
        printf("Delay Min: %d\n" , cfg.delay_min) ;  //Print minimum delay
        printf("Delay Max: %d\n" , cfg.delay_max) ;  //Print maximum delay
        printf("Rounds to Win: %d\n" , cfg.rounds_to_win) ;  //Print rounds to win

        printf("-------------------------------------\n") ;

        break ;  //Exit loop since everything is valid
    }

    // ---------------------------------------------
    // Final message to confirm successful loading 
    // ---------------------------------------------
    printf("\nProgram loaded successfully.\n") ;

    init_furniture_rng() ; //Initialize random number generator for furniture setup

    int mode ;  //Variable to store generation mode choice
    printf("\n1) Random\n2) Custom Range\nChoice: ") ;  //Ask user to choose generation mode
    scanf("%d" , &mode) ;  //Read user choice for generation mode

    int min = 0 , max = 0 ;  //Variables to store min and max values if range mode is selected

    if (mode == 2)
    {
        printf("Enter min: ") ;
        scanf("%d" , &min) ;
        printf("Enter max: ") ;
        scanf("%d" , &max) ;
    }
    
    FurnitureSet set ;  //Declare a FurnitureSet struct to hold the generated pieces

    if (init_furniture_set(&set , cfg.num_pieces , mode , min , max) != 0)  //Initialize the furniture set based on user configuration and generation mode, check for errors
    {
        printf("Error: invalid range! Not enough unique values for %d pieces\n" , cfg.num_pieces) ;
        exit(1) ;  //Exit with error if furniture initialization fails
    }

    //Print a samples for verification:
    printf("\n--- Furniture Samples Generation Test---\n") ;
    print_sample("Set of Team 0" , set.teamA , set.countA , 10) ; 
    print_sample("Set of Team 1" , set.teamB , set.countB , 10) ;
    print_sample("Sorted Set" , set.sorted , cfg.num_pieces , 10) ;
    
    Pipes p ;  //Declare a Pipes struct to hold the created pipes for testing

    if (init_pipes(&p , cfg.num_workers) != 0)  //Initialize pipes based on the number of workers specified in the configuration, check for errors
    {
        printf("Error creating pipes\n") ;
        exit(1) ;  //Exit with error if pipe initialization fails
    }

    printf("\n--- Pipes Test ---\n") ;

    for (int i = 0 ; i < cfg.num_workers - 1 ; i++)  //Loop through each pipe and print their file descriptors for verification
    {
        printf("Forward[%d]: R=%d W=%d\n" , i , p.forward[i][0] , p.forward[i][1]) ;  //Print read and write file descriptors for forward pipe i
        printf("Backward[%d]: R=%d W=%d\n" , i , p.backward[i][0] , p.backward[i][1]) ;  //Print read and write file descriptors for backward pipe i
    }

    free_pipes(&p) ;  //Free allocated pipe memory after testing

    run_controller(&cfg , mode , min , max) ;  //Run the main controller logic to start the game with the loaded configuration

    return 0 ;  //Return success status to OS
}