#include <stdio.h>   
#include <stdlib.h>  
#include <string.h> 
#include <limits.h>  //Provides INT_MAX and INT_MIN for integer overflow checking
#include "config.h"  
#include "utils.h"  

// =============================
// Validate logical constraints
// =============================
void validate_config(Config *cfg)  //Function takes pointer to Config struct to validate values
{
    if (cfg->num_workers < 2)  //Check if number of workers is less than minimum allowed (2)
        error_exit("num_workers must be >= 2") ;  //Exit program with error message

    if (cfg->num_pieces <= 1)  //Check if number of pieces is less than or equal to 1
        error_exit("num_pieces must be > 1") ;  //Exit if invalid

    if (cfg->delay_min <= 0)  //Check if minimum delay is not strictly positive
        error_exit("delay_min must be > 0") ;  //Exit if invalid

    if (cfg->delay_max < cfg->delay_min)  //Ensure max delay is not smaller than min delay
        error_exit("delay_max must be >= delay_min") ;  //Exit if invalid

    if (cfg->rounds_to_win <= 0)  //Check if rounds to win is not positive
        error_exit("rounds_to_win must be > 0") ;  //Exit if invalid
}

// ==========================================
// Load and strictly parse configuration file
// ==========================================
Config load_config(const char *filename)  //Function receives file name and returns Config struct
{
    FILE *file = fopen(filename , "r") ;  // Attempt to open file in read mode

    if (!file)  //If file pointer is NULL (open failed)
    {
        perror("Error opening config file") ;  //Print system error message
        exit(EXIT_FAILURE) ;  //Terminate program with failure status
    }

    Config cfg = {0} ;  //Initialize all struct fields to 0 (safe default)

    char line[256] ;  //Buffer to store each line read from file
    int line_num = 0 ;  //Counter to track line number for error reporting

    int found[5] = {0} ;  //Array to track if each key appeared (detect duplicates)
    int has_data = 0 ;  //Flag to detect if file has any non-empty lines
    int valid_lines = 0 ;  //Counter for number of valid (non-empty) lines

    while (fgets(line , sizeof(line) , file))   //Read file line by line until EOF
    {
        line_num++ ;  //Increment line number

        trim_newline(line) ;  //Remove newline character '\n' from the end

        if (strlen(line) == 0)  //If line is empty after trimming
            continue ;  //Skip this line and continue loop

        has_data = 1 ;  //Mark that file contains at least one valid line
        valid_lines++ ;  //Increment count of valid lines

        // ==========================
        // Enforce NO SPACES rule
        // ==========================
        for (int i = 0 ; line[i] ; i++)  //Loop through each character in the line
        {
            if (line[i] == ' ' || line[i] == '\t')  //If space or tab is found
            {
                printf("Error (line %d): Spaces are not allowed\n" , line_num) ;  //Print error
                exit(EXIT_FAILURE) ;  //Terminate program
            }
        }

        char key[64] ;  //Buffer to store key (left side of '=')
        char value_str[64] ;  //Buffer to store value as string (right side of '=')

        // ==========================
        // Strict format: key=value
        // ==========================
        if (sscanf(line, "%[^=]=%s" , key , value_str) != 2)  //Parse key and value
        {
            printf("Error (line %d): Invalid format. Use key=value\n" , line_num) ;  //Error if parsing fails
            exit(EXIT_FAILURE) ;  //Terminate program
        }

        // ==========================
        // Safe integer parsing
        // ==========================
        char *endptr ;  //Pointer used by strtol to detect invalid characters

        long val = strtol(value_str , &endptr , 10) ;  //Convert string to long integer (base 10)

        if (*endptr != '\0')  //If extra characters exist after number (e.g., "123abc")
        {
            printf("Error (line %d): Value must be integer\n" , line_num) ;  //Invalid integer
            exit(EXIT_FAILURE) ;  //Terminate program
        }

        if (val > INT_MAX || val < INT_MIN)  //Check if value exceeds int range[-2147483647 , 2147483647]
        {
            printf("Error (line %d): Integer value out of range\n" , line_num) ;  //Overflow/underflow
            exit(EXIT_FAILURE) ;  //Terminate program
        }

        int value = (int)val ;  //Safely cast long to int after validation

        // ====================================
        // Assign values and detect duplicates
        // ====================================

        if (strcmp(key , "num_workers") == 0) //If key matches "num_workers"
        {
            if (found[0])  //Check if already assigned before
            {
                printf("Error (line %d): Duplicate key 'num_workers'\n" , line_num) ;  //Duplicate error
                exit(EXIT_FAILURE) ;  //Terminate program
            }
            cfg.num_workers = value ;  //Assign value to struct field
            found[0] = 1 ;  //Mark this key as found
        }
        else if (strcmp(key , "num_pieces") == 0)  //Check for "num_pieces"
        {
            if (found[1])
            {
                printf("Error (line %d): Duplicate key 'num_pieces'\n" , line_num) ;
                exit(EXIT_FAILURE) ;
            }
            cfg.num_pieces = value ;
            found[1] = 1 ;
        }
        else if (strcmp(key , "delay_min") == 0)  //Check for "delay_min"
        {
            if (found[2])
            {
                printf("Error (line %d): Duplicate key 'delay_min'\n" , line_num) ;
                exit(EXIT_FAILURE) ;
            }
            cfg.delay_min = value ;
            found[2] = 1 ;
        }
        else if (strcmp(key , "delay_max") == 0)  //Check for "delay_max"
        {
            if (found[3])
            {
                printf("Error (line %d): Duplicate key 'delay_max'\n", line_num) ;
                exit(EXIT_FAILURE) ;
            }
            cfg.delay_max = value ;
            found[3] = 1 ;
        }
        else if (strcmp(key , "rounds_to_win") == 0) //Check for "rounds_to_win"
        {
            if (found[4])
            {
                printf("Error (line %d): Duplicate key 'rounds_to_win'\n", line_num) ;
                exit(EXIT_FAILURE) ;
            }
            cfg.rounds_to_win = value ;
            found[4] = 1 ;
        }
        else  //If key does not match any known variable
        {
            printf("Error (line %d): Unknown key '%s'\n" , line_num , key) ;  //Unknown key error
            exit(EXIT_FAILURE) ;  //Terminate program
        }
    }

    fclose(file) ;  //Close file after finishing reading

    // ==========================
    // Check empty file
    // ==========================
    if (!has_data)  //If no valid lines were found
    {
        error_exit("Config file is empty") ;  //Exit with error
    }

    // ==========================
    // Reject extra lines (>5)
    // ==========================
    if (valid_lines > 5)  //If more than expected number of lines
    {
        error_exit("Too many lines in config file (expected exactly 5)") ;  //Error
    }

    // ==========================
    // Check missing keys
    // ==========================
    const char *names[] = {"num_workers" , "num_pieces" , "delay_min" , "delay_max" , "rounds_to_win"} ;  //Array of required key names

    int missing = 0 ;  //Flag to detect if any key is missing

    for (int i = 0 ; i < 5 ; i++)  //Loop through all expected keys
    {
        if (!found[i])  //If key was not found
        {
            printf("Error: Missing key '%s'\n", names[i]) ;  //Print missing key
            missing = 1 ;  //Mark error
        }
    }

    if (missing)  //If any key was missing
        exit(EXIT_FAILURE) ;  //Terminate program

    // ==========================
    // Final logical validation
    // ==========================
    validate_config(&cfg) ;  //Validate logical correctness of values

    return cfg ;  //Return fully validated configuration struct
}