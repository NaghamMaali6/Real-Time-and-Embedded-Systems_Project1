#ifndef CONFIG_H
#define CONFIG_H

//Struct that holds ALL configuration values read from file:
typedef struct 
{
    int num_workers ;  //number of workers per team (must be >= 2)
    int num_pieces ;  //total furniture pieces (must be > 0)
    int delay_min ;  //minimum delay (must be > 0)
    int delay_max ;  //maximum delay (must be >= delay_min)
    int rounds_to_win ;  //number of rounds to win (must be > 0)
} 
Config ;

Config load_config(const char *filename) ;  //Reads config file, parses it, validates it, and returns filled struct

void validate_config(Config *cfg) ;  //Checks logical correctness of values

#endif