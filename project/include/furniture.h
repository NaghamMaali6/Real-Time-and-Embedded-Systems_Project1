#ifndef FURNITURE_H
#define FURNITURE_H

typedef struct
{
    int *teamA ;  //shuffled pieces for team A
    int *teamB ;  //shuffled pieces for team B
    int *sorted ;  //correct order reference
    int countA ;  //number of pieces in team A
    int countB ;  //number of pieces in team B
} 
FurnitureSet ;

void init_furniture_rng() ;  //RNG initialization

//Generation modes:
int generate_random_unique(int n , int *out) ;  //Generate random unique numbers by the system
int generate_range_unique(int n , int min , int max , int *out) ;  //Generate unique numbers from specified range

int init_furniture_set(FurnitureSet *set , int n , int mode , int min , int max) ;  //Setup full furniture set with specified mode and parameters

void free_furniture_set(FurnitureSet *set) ;  //Cleanup: Free all dynamically allocated memory in the set

void print_sample(const char *label , int *arr , int n , int max_print) ;  //print sample of array for testing

int pick_random_piece(FurnitureSet *set, int last_failed , int team_id) ;  //Pick a random piece from the furniture set, avoiding the last failed piece to increase chances of acceptance

void remove_piece(FurnitureSet *set, int value , int team_id) ;  //Remove a piece from the furniture set after it has been accepted to prevent it from being picked again

#endif