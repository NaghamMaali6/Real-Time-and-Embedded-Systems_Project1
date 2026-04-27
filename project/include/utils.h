#ifndef UTILS_H
#define UTILS_H

void error_exit(const char *message) ;  //Print error message and terminate program

void trim_newline(char *str) ;  //Remove newline '\n' from end of string (if exists)

int random_range(int min , int max) ;  //Generate a random integer in the range [min , max]

void safe_close(int fd) ; //Close file descriptor safely if valid

#endif