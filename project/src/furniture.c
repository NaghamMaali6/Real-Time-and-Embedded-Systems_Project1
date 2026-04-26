#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "furniture.h"

// ======================
// Fisher-Yates Shuffle
// ======================
static void shuffle(int *arr , int n)
{
    for (int i = n - 1; i > 0; i--)  //Iterate backwards through the array
    {
        int j = rand() % (i + 1) ;  //Generate random index from 0 to i

        int tmp = arr[i] ;  //Store element at position i
        arr[i] = arr[j] ;  //Move element at j to i
        arr[j] = tmp ;  //Move original element at i to j
    }
}

// ====================
// RNG initialization
// ====================
void init_furniture_rng()
{
    srand((unsigned int)(time(NULL) ^ getpid())) ;  //Initialize random number generator
}

// ===============================
// Generate [1..N] shuffled
// ===============================
int generate_random_unique(int n , int *out)
{
    if (!out || n <= 0) return -1 ;  //Validate output pointer and n

    for (int i = 0 ; i < n ; i++)
        out[i] = i + 1 ;  //Fill array with sequential numbers starting from 1

    shuffle(out , n) ;  //Shuffle the array to create a random unique sequence

    return 0 ;
}

// ===================================
// Generate unique numbers from range
// ===================================
int generate_range_unique(int n , int min , int max , int *out)
{
    if (!out || n <= 0 || min > max) return -1 ;  //Validate parameters

    int range = max - min + 1 ;  //Calculate the range of possible values
    if (range < n) return -1 ;  //If range is smaller than n, it's impossible to generate unique numbers

    int *pool = malloc(range * sizeof(int)) ;  //Create a pool of all possible numbers in the range
    if (!pool) return -1 ;  //Check for memory allocation failure

    for (int i = 0 ; i < range ; i++)
        pool[i] = min + i ;  //Fill pool with all numbers from min to max

    shuffle(pool , range) ;  //Shuffle the pool to randomize the order of numbers

    for (int i = 0 ; i < n ; i++)
        out[i] = pool[i] ;  //Take the first n numbers from the shuffled pool as output

    free(pool) ;  //Free the pool memory after use
    return 0 ;
}

// ===============================
// Initialize FULL furniture set
// ===============================
int init_furniture_set(FurnitureSet *set , int n , int mode , int min , int max)
{
    if (!set || n <= 0) return -1 ;  //Validate input parameters

    set->count = n ;  //Set the count of pieces for each team

    set->teamA = malloc(n * sizeof(int)) ;  //Allocate memory for team A pieces
    set->teamB = malloc(n * sizeof(int)) ;  //Allocate memory for team B pieces
    set->sorted = malloc(n * sizeof(int)) ;  //Allocate memory for sorted reference

    if (!set->teamA || !set->teamB || !set->sorted)  //Check for memory allocation failure
        return -1 ;

    int *base = malloc(n * sizeof(int)) ;  //generate base data
    if (!base) return -1 ;

    if (mode == 1)
    {
        if (generate_random_unique(n , base) != 0)  //Generate random unique numbers
            return -1 ;
    }
    else
    {
        if (generate_range_unique(n , min , max , base) != 0)  //Generate unique numbers from a specified range
            return -1 ;
    }

    //copy into both teams:
    for (int i = 0 ; i < n ; i++)
    {
        set->teamA[i] = base[i] ;  //Copy base data to team A
        set->teamB[i] = base[i] ;  //Copy base data to team B
    }

    //shuffle independently for fairness:
    shuffle(set->teamA, n) ;  //Shuffle team A pieces
    shuffle(set->teamB, n) ;  //Shuffle team B pieces

    for (int i = 0 ; i < n ; i++)
        set->sorted[i] = base[i] ;  //create sorted reference by copying base data first

    //sort reference(bubble sort):
    for (int i = 0 ; i < n - 1 ; i++)
    {
        for (int j = i + 1 ; j < n ; j++)
        {
            if (set->sorted[i] > set->sorted[j])
            {
                int tmp = set->sorted[i] ;  //Swap elements to sort the reference array
                set->sorted[i] = set->sorted[j] ;  //Move smaller element to position i
                set->sorted[j] = tmp ;  //Move larger element to position j
            }
        }
    }

    free(base) ;  //Free the temporary base array after use
    return 0 ;
}

// =============
// Free memory
// =============
void free_furniture_set(FurnitureSet *set)
{
    if (!set) return ;  //Validate input pointer

    free(set->teamA) ;  //Free memory allocated for team A pieces
    free(set->teamB) ;  //Free memory allocated for team B pieces
    free(set->sorted) ;
}

// ============
// Debug print
// ============
void print_sample(const char *label , int *arr , int n , int max_print)
{
    printf("%s: " , label) ;

    int limit = (n < max_print) ? n : max_print ;  //Determine the number of elements to print

    for (int i = 0 ; i < limit ; i++)
        printf("%d " , arr[i]) ;  //Print each element followed by a space

    printf("\n") ;
}

// ======================================
// Pick random piece (avoid last_failed)
// ======================================
int pick_random_piece(FurnitureSet *set, int last_failed)
{
    if (!set || set->count <= 0)
        return -1;

    int index;

    do
    {
        index = rand() % set->count;
    }
    while (set->count > 1 && set->teamA[index] == last_failed);

    return set->teamA[index];
}

// ======================================
// Remove piece from team
// ======================================
void remove_piece(FurnitureSet *set , int value)
{
    if (!set || set->count <= 0)  //Validate input parameters and check if there are pieces to remove
        return ;

    int found = -1 ;  //Initialize found flag(-1 : not found)

    for (int i = 0 ; i < set->count ; i++)  //Search for the piece to remove
    {
        if (set->teamA[i] == value)
        {
            found = i ;  //Mark the index of the found piece
            break ;
        }
    }

    if (found == -1) 
        return ;  //If piece not found, exit the function

    //shift left:
    for (int i = found ; i < set->count - 1 ; i++)
    {
        set->teamA[i] = set->teamA[i + 1] ;  //Shift pieces to the left to fill the gap left by the removed piece
    }

    set->count-- ;  //Decrease the count of pieces after removal
}