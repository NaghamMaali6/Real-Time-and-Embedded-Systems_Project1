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

    set->countA = n ;  //Initialize the count of pieces for team A
    set->countB = n ;  //Initialize the count of pieces for team B

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
int pick_random_piece(FurnitureSet *set , int last_failed , int team_id)
{
    if (!set)  //Validate input pointer
        return -1 ;

    int *arr ;  //Pointer to the array of pieces for the selected team
    int *count ;  //Pointer to the count of pieces for the selected team

    //Select correct team:
    if (team_id == 0)
    {
        arr = set->teamA ;  //Set arr to point to team A pieces
        count = &set->countA ;  //Set count to point to the count of pieces for team A
    }
    else
    {
        arr = set->teamB ;  //Set arr to point to team B pieces
        count = &set->countB ;  //Set count to point to the count of pieces for team B
    }

    if (*count <= 0)  //Check if there are any pieces left to pick
        return -1 ;  //Return -1 if there are no pieces left to pick

    int index ;  //Variable to store the randomly selected index of the piece to pick

    do
    {
        index = rand() % (*count) ;  //Generate a random index to pick a piece from the team's array of pieces
    }
    while (*count > 1 && arr[index] == last_failed) ;  //If there is more than one piece left and the randomly selected piece is the same as the last failed piece, repeat the random selection to increase chances of picking a different piece that may be accepted

    return arr[index] ;  //Return the randomly selected piece from the team's array of pieces
}

// =======================
// Remove piece from team
// =======================
void remove_piece(FurnitureSet *set , int value , int team_id)
{
    if (!set)  //Validate input pointer
        return ;  

    int *arr ;  //Pointer to the array of pieces for the selected team
    int *count ;  //Pointer to the count of pieces for the selected team

    //Select correct team:
    if (team_id == 0)  //Check if team ID is 0 (Team A)
    {
        arr = set->teamA ;  //Set arr to point to team A pieces
        count = &set->countA ;  //Set count to point to the count of pieces for team A
    }
    else
    {
        arr = set->teamB ;  //Set arr to point to team B pieces
        count = &set->countB ;  //Set count to point to the count of pieces for team B
    }

    if (*count <= 0)  //Check if there are any pieces left to remove
        return ;  //Return if there are no pieces left to remove

    int found = -1 ;  //Variable to store the index of the piece to remove, initialized to -1 to indicate not found

    for (int i = 0 ; i < *count ; i++)  //Iterate through the array of pieces for the selected team
    {
        if (arr[i] == value)  //If the piece to remove is found
        {
            found = i ;  //If the piece to remove is found, store its index in the found variable
            break ;  //Exit the loop since we found the piece to remove
        }
    }

    if (found == -1)  //If the piece to remove was not found in the team's array of pieces, return without making any changes
        return ;  

    //shift left:
    for (int i = found ; i < *count - 1 ; i++)
    {
        arr[i] = arr[i + 1] ;  //Shift elements to the left to fill the gap left by the removed piece, effectively removing it from the array
    }

    (*count)-- ;  //Decrement the count of pieces for the selected team since one piece has been removed
}