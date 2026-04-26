#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "utils.h"

void error_exit(const char *message)  //Print error and exit function
{
    fprintf(stderr, "%s\n" , message) ;  //Print message to stderr (standard error stream)

    exit(EXIT_FAILURE) ;  //Immediately terminate program with failure status
}

void trim_newline(char *str)  //Remove newline character from string function
{
    //Loop through characters until end of string:
    for (int i = 0 ; str[i] != '\0' ; i++) 
    {
        if (str[i] == '\n')  //If newline is found
        {
            str[i] = '\0' ;  //Replace it with string terminator

            return ;
        }
    }
}

int random_range(int min , int max)
{
    return min + rand() % (max - min + 1) ;  //Generate random number in range [min , max]
}

void safe_close(int fd)
{
    if (fd >= 0)  //If file descriptor is valid (non-negative)
        close(fd) ;
}