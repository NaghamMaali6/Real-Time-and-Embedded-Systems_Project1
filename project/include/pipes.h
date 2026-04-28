#ifndef PIPES_H
#define PIPES_H

typedef struct 
{
    int W ;
    int (*forward)[2] ;
    int (*backward)[2] ;
} 
Pipes ;

int init_pipes(Pipes *p , int W) ;  //Initialize pipes for W workers (W-1 pairs)
void free_pipes(Pipes *p) ;  //Free allocated pipe memory and close file descriptors

#endif