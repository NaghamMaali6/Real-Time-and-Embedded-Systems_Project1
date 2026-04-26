#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "pipes.h"

int init_pipes(Pipes *p , int W)
{
    p->W = W ;  //Store the number of workers in the Pipes struct

    p->forward = malloc((W - 1) * sizeof(int[2])) ;  //Allocate memory for forward pipes (W-1 pipes needed to connect W workers)
    p->backward = malloc((W - 1) * sizeof(int[2])) ;  //Allocate memory for backward pipes (W-1 pipes needed for return communication)

    if (!p->forward || !p->backward)  //Check for memory allocation failure
        return -1 ;

    for (int i = 0 ; i < W - 1 ; i++)  //Create pipes for forward and backward communication between workers
    {
        if (pipe(p->forward[i]) == -1)
            return -1 ;  //If forward pipe creation fails, return error

        if (pipe(p->backward[i]) == -1)
            return -1 ; //If backward pipe creation fails, return error
    }

    return 0 ;
}

void free_pipes(Pipes *p)
{
    if (!p) return ;  //Validate input pointer

    for (int i = 0 ; i < p->W - 1 ; i++)  //Close all pipe file descriptors before freeing memory
    {
        close(p->forward[i][0]) ;  //Close read end of forward pipe
        close(p->forward[i][1]) ;  //Close write end of forward pipe
        close(p->backward[i][0]) ;  //Close read end of backward pipe
        close(p->backward[i][1]) ;  //Close write end of backward pipe
    }

    free(p->forward) ;  //Free memory allocated for forward pipes
    free(p->backward) ;  //Free memory allocated for backward pipes
}