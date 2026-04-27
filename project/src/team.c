#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "team.h"
#include "worker.h"
#include "pipes.h"
#include "furniture.h"

// =====================================
// Start a team (creates workers + pipes)
// =====================================
void start_team(Config *cfg , FurnitureSet *furniture , int round , int team_id)
{
    Pipes pipes ;

    //Initialize pipes:
    if (init_pipes(&pipes , cfg->num_workers) != 0)  //Check for errors during pipe initialization and exit if any issues occur
    {
        perror("pipes") ;
        exit(1) ;  //Exit with error code if pipe initialization fails to ensure proper error handling and prevent undefined behavior
    }

    //Create worker processes:
    for (int i = 0 ; i < cfg->num_workers ; i++)
    {
        pid_t pid = fork() ;  //Fork a new process for each worker to enable concurrent processing of pieces in the assembly line

        if (pid < 0)  //Check for errors during fork and exit if any issues occur to ensure proper error handling and prevent undefined behavior
        {
            perror("fork") ;  //Report the error
            exit(1) ;  //Exit with error code if fork fails to ensure proper error handling and prevent undefined behavior
        }

        //=======================
        // Child process: worker
        //=======================
        if (pid == 0)  //In the child process, call the worker_process function with the appropriate parameters to start the worker's execution and then exit to prevent unintended code execution in the child process
        {
            worker_process(i , &pipes , cfg , furniture , round , team_id) ;  //Call the worker_process function with the worker ID, pipes, configuration, furniture set, current round, and team ID to start the worker's execution and handle the assembly line processing for the assigned pieces
            exit(0) ;  //safety exit
        }
    }

    //Parent waits for all workers:
    for (int i = 0 ; i < cfg->num_workers ; i++)
    {
        wait(NULL) ;  //Wait for each worker process to finish to ensure that the parent process does not exit prematurely and allows all worker processes to complete their tasks before proceeding with cleanup and termination of the team
    }

    free_pipes(&pipes) ;  //Clean up pipes after all workers have finished to free allocated resources and prevent memory leaks, ensuring proper cleanup of the pipes used for inter-process communication in the team
}