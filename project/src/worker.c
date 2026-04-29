#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "worker.h"
#include "furniture.h"
#include <signal.h>

static volatile sig_atomic_t terminate_flag = 0 ;  //Global termination flag updated by signal handler ensures safe access between signal context and normal execution

#define ACCEPT 0  //Define ACCEPT as 0 for clarity in communication between processes
#define REJECT 1  //Define REJECT as 1 for clarity in communication between processes

#define TERMINATE -1  //Define TERMINATE as -1 to signal the end of processing to middle workers

#define PRINT_IF_ALIVE(...) do { if (!terminate_flag) printf(__VA_ARGS__) ; } while(0)  //Macro to safely print only if the worker is still active to Prevents noisy or inconsistent output after a SIGTERM is received

// ===========================
// Signal handler for SIGTERM
// ===========================
static void handle_sigterm(int sig)  //function to Set termination flag instead of exiting immediately to allow graceful shutdown
{
    (void)sig ;  //suppress unused parameter warning
    terminate_flag = 1 ;  //notify worker loops to exit safely
}

// ====================
// Delay with fatigue
// ====================
static void do_delay(Config *cfg , int round , int *last_delay)
{
    int base = rand() % (cfg->delay_max - cfg->delay_min + 1) + cfg->delay_min ;  //Generate random base delay within the specified range
    int fatigue = round * 2 ;  //Calculate fatigue based on the current round (increasing delay as rounds progress)
    int delay = base + fatigue ;  //Total delay is the sum of base delay and fatigue

    if (delay < *last_delay)  //Ensure that the delay does not decrease compared to the last delay (simulate fatigue)
        delay = *last_delay ;  //Use the last delay if the new delay is smaller

    *last_delay = delay ;  //Update last delay for the next call

    usleep(delay * 1000) ;  //Sleep for the calculated delay (convert milliseconds to microseconds)
}

// ===================
// Close unused pipes
// ===================
static void close_unused_pipes(Pipes *pipes , int id , int workers)
{
    for (int i = 0 ; i < workers - 1 ; i++) 
    {
        if (id != i + 1) close(pipes->forward[i][0]) ;  //Forward read: only worker (i+1) reads forward[i][0]
        
        if (id != i) close(pipes->forward[i][1]) ;  //Forward write: only worker i writes forward[i][1]
        
        if (id != i) close(pipes->backward[i][0]) ;  //Backward read: only worker i reads backward[i][0]
        
        if (id != i + 1) close(pipes->backward[i][1]) ;  //Backward write: only worker (i+1) writes backward[i][1]
    }
}

// ===============
// Worker process
// ===============
void worker_process(int id , Pipes *pipes , Config *cfg , FurnitureSet *furniture , int round , int team_id)
{
    srand(getpid()) ;  //Seed the random number generator with the process ID for unique sequences in each worker

    struct sigaction sa ;  //Structure used to define how the process handles signals
    sa.sa_handler = handle_sigterm ;  //Assign custom handler: instead of terminating immediately
    sigemptyset(&sa.sa_mask) ;  //Do not block any additional signals while handling SIGTERM, allowing minimal interference with normal execution
    sa.sa_flags = 0 ;  //let read/write get interrupted by signals (EINTR)
    sigaction(SIGTERM , &sa , NULL) ;  //Register the handler for SIGTERM signal using POSIX API
    
    int *myCount = (team_id == 0) ? &furniture->countA : &furniture->countB ;  //Select the correct piece counter for the current team using a pointer(Point to this team's remaining pieces counter (countA or countB))
    //note: myCount is a POINTER so: myCount = address of the counter and *myCount = actual number of remaining pieces

    close_unused_pipes(pipes , id , cfg->num_workers) ;  //Close unused pipe ends to prevent resource leaks and ensure proper communication

    int last_delay = 0 ;  //Initialize last delay to 0 for the first call to do_delay (no fatigue on the first round)

    // ================
    // SOURCE (id = 0)
    // ================
    if (id == 0)
    {
        int last_failed = -1 ;  //Initialize last failed piece to -1 (indicating no failed piece at the start)

        while (*myCount > 0 && !terminate_flag)  //Continue picking pieces until all pieces have been successfully sent and accepted and terminate_flag = 1
        {
            do_delay(cfg , round , &last_delay) ;  //Simulate processing delay with fatigue for the source worker before picking the next piece to send, allowing for a more realistic simulation of the assembly line process and increasing the chances of acceptance by avoiding immediate retries of failed pieces
            
            int piece = pick_random_piece(furniture , last_failed , team_id) ;  //Pick a random piece from the furniture set, avoiding the last failed piece to increase chances of acceptance

            PRINT_IF_ALIVE("[Team %d - SOURCE] sending %d\n" , team_id , piece) ;  //Log the piece being sent for debugging and tracking purposes

            write(pipes->forward[0][1] , &piece , sizeof(int)) ;  //Send the picked piece to the next worker through the forward pipe

            int result ;  //Initialize result variable to store the acceptance/rejection status
            
            ssize_t n = read(pipes->backward[0][0] , &result , sizeof(int)) ;  //Read the result from the next worker through the backward pipe
            if (n <= 0) break ;  //if pipe closed or error, exit loop

            if (result == ACCEPT)
            {
                PRINT_IF_ALIVE("[Team %d - SOURCE] accepted %d\n" , team_id , piece) ;  //Log acceptance for debugging and tracking purposes
                remove_piece(furniture , piece , team_id) ;  //Remove the accepted piece from the furniture set to prevent it from being picked again
                last_failed = -1 ;  //Reset last failed piece since the current piece was accepted
            }
            else
            {
                PRINT_IF_ALIVE("[Team %d - SOURCE] rejected %d by the sink so I put it back\n" , team_id , piece) ;  //Log rejection for debugging and tracking purposes
                last_failed = piece ;  //Update last failed piece to the current piece to avoid picking it again immediately, increasing chances of acceptance in the next attempt
            }
        }

        //after finishing all pieces:
        int term = TERMINATE ;  
        write(pipes->forward[0][1] , &term , sizeof(int)) ;  //Send termination signal to the next worker to indicate that there are no more pieces to process
        
        //CLOSE ALL USED ENDS:
        close(pipes->forward[0][1]) ;   
        close(pipes->backward[0][0]) ;
        
        exit(0) ;  //Exit the source worker process after sending the termination signal
    }

    // ===================
    // SINK (last worker)
    // ===================
    else if (id == cfg->num_workers - 1)
    {
        int index = 0 ;  //Initialize index to track the position in the sorted reference array for validation of received pieces

        while (!terminate_flag)
        {
            int piece = -1 ;  //Initialize piece variable to store the received piece from the previous worker
            
            ssize_t n = read(pipes->forward[id - 1][0] , &piece , sizeof(int)) ;  //Read the piece sent by the previous worker through the forward pipe
            if (n <= 0) break ;  //if pipe closed or error, exit loop
            
            if (piece == TERMINATE)  //Exit the sink worker process after successfully validating all pieces in the sorted reference array
            {
                break ;
            }

            do_delay(cfg , round , &last_delay) ;  //Simulate processing delay with fatigue for the sink worker before validating the piece

            PRINT_IF_ALIVE("[Team %d - SINK] got %d | expected %d\n" , team_id , piece , furniture->sorted[index]) ;  //Log the received piece and the expected piece for debugging and tracking purposes

            int result ;

            if (piece == furniture->sorted[index])  //Check if the received piece matches the expected piece in the sorted reference array
            {
                PRINT_IF_ALIVE("[Team %d - SINK] ACCEPT\n" , team_id) ;  //Log acceptance for debugging and tracking purposes
                result = ACCEPT ;  //Set result to ACCEPT if the received piece matches the expected piece in the sorted reference array
                index++ ;  //Move to the next expected piece in the sorted reference array for the next validation
            }
            else
            {
                PRINT_IF_ALIVE("[Team %d - SINK] REJECT\n" , team_id) ;  //Log rejection for debugging and tracking purposes
                result = REJECT ;  //Set result to REJECT if the received piece does not match the expected piece in the sorted reference array
            }

            write(pipes->backward[id - 1][1] , &result , sizeof(int)) ;  //Send the acceptance/rejection result back to the previous worker through the backward pipe

            if (index >= cfg->num_pieces)
            {
                break ;
            }
        }

        //CLOSE ALL USED ENDS:
        close(pipes->forward[id - 1][0]) ;  
        close(pipes->backward[id - 1][1]) ;

        exit(0) ;
    }

    // ===============
    // MIDDLE workers
    // ===============
    else
    {
        while (!terminate_flag)
        {
            int piece = -1 ;  //Initialize piece variable to store the received piece from the previous worker

            ssize_t n1 = read(pipes->forward[id - 1][0] , &piece , sizeof(int)) ;  //Read the piece sent by the previous worker through the forward pipe
            if (n1 <= 0) break ;  //if pipe closed or error, exit loop

            if (piece == TERMINATE)  //Check for termination signal from the previous worker
            {
                write(pipes->forward[id][1] , &piece, sizeof(int)) ;  //Forward the termination signal to the next worker to propagate the end of processing
                break ;  //exit loop
            }

            do_delay(cfg , round , &last_delay) ;  //Simulate processing delay with fatigue for middle workers before forwarding the piece to the next worker

            PRINT_IF_ALIVE("[Team %d - MIDDLE %d] passing forward %d\n" , team_id , id , piece) ;  //Log the piece being forwarded for debugging and tracking purposes

            write(pipes->forward[id][1] , &piece , sizeof(int)) ;  //Forward the received piece to the next worker through the forward pipe

            int result ;  //Initialize result variable to store the acceptance/rejection result

            ssize_t n2 = read(pipes->backward[id][0] , &result , sizeof(int)) ;  //Read the acceptance/rejection result from the next worker through the backward pipe
            if (n2 <= 0) break ;  //if pipe closed or error, exit loop

            do_delay(cfg , round , &last_delay) ;  //Simulate processing delay with fatigue for middle workers before sending the result back to the previous worker

            if(result == REJECT)
            {
            PRINT_IF_ALIVE("[Team %d - MIDDLE %d] passing backward %d\n" , team_id , id , piece) ;  //Log the piece being sent back for debugging and tracking purposes
            }
            else if(result == ACCEPT)
            {
                PRINT_IF_ALIVE("[Team %d - MIDDLE %d] notification of acceptance of %d\n" , team_id , id , piece) ;  //Log the piece accepted(informing source)
            }
            
            write(pipes->backward[id - 1][1] , &result , sizeof(int)) ;  //Send the acceptance/rejection result back to the previous worker through the backward pipe
        }  

        //CLOSE ALL USED ENDS:
        close(pipes->forward[id - 1][0]) ; 
        close(pipes->forward[id][1]) ;  
        close(pipes->backward[id][0]) ;
        close(pipes->backward[id - 1][1]) ; 

        exit(0) ;  
    }
}