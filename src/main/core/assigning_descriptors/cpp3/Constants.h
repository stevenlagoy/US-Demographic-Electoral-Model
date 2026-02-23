#ifndef CONSTANTS_H
#define CONSTANTS_H

// ANSI Color Codes
#define ESC "\033"
#define CSI "["
#define SGR "m"
#define CUP "H"
#define EL "K"
#define ALL "2"
#define RESET ESC CSI "0" SGR
#define SEP ";"

#define CLEAR_LINE ESC CSI ALL EL
#define HIDE_CURSOR ESC CSI "?251"

#define FG_BLACK "30"
#define FG_RED "31"
#define FG_GREEN "32"
#define FG_YELLOW "33"
#define FG_BLUE "34"
#define FG_MAGENTA "35"
#define FG_CYAN "36"
#define FG_WHITE "37"

#define FG_BRIGHT_BLACK "90"
#define FG_BRIGHT_RED "91"
#define FG_BRIGHT_GREEN "92"
#define FG_BRIGHT_YELLOW "93"
#define FG_BRIGHT_BLUE "94"
#define FG_BRIGHT_MAGENTA "95"
#define FG_BRIGHT_CYAN "96"
#define FG_BRIGHT_WHITE "97"

#define BG_BLACK "40"
#define BG_RED "41"
#define BG_GREEN "42"
#define BG_YELLOW "43"
#define BG_BLUE "44"
#define BG_MAGENTA "45"
#define BG_CYAN "46"
#define BG_WHITE "47"

#define BG_BRIGHT_BLACK "100"
#define BG_BRIGHT_RED "101"
#define BG_BRIGHT_GREEN "102"
#define BG_BRIGHT_YELLOW "103"
#define BG_BRIGHT_BLUE "104"
#define BG_BRIGHT_MAGENTA "105"
#define BG_BRIGHT_CYAN "106"
#define BG_BRIGHT_WHITE "107"

#define NATIONAL_DESCRIPTOR_NAME "$$$$USA"

#define REGION_DESCRIPTOR_PREFIX "$$$"

#define DIVISION_DESCRIPTOR_PREFIX "$$"

#define STATE_DESCRIPTOR_PREFIX "$"

// The number of demographic keys used in the dataset
#define NUMBER_DEMOGRAPHICS 223
// The number of descriptors we will create (including fixed-membership [USA, AL, etc])
#define NUMBER_DESCRIPTORS 750 // UNUSED
// Maximum change (positive or negative) to a descriptor effect from one permutation - percentage
#define MAX_CHANGE_AMT 0.25
// Approximate number of counties: DO NOT USE AS EXACT
#define NUMBER_COUNTIES 3100

// Percentage compared to national average at which a demographic modifier is considered impactful
#define IMPACTFUL_DEMOGRAPHIC_BOUNDARY 0.01

#define MAX_ITERATIONS 100'000'000
#define MAX_TRIES 1000000

// Temperature is the probability that a worse change is accepted
#define STARTING_TEMPERATURE 1.0
#define TEMPERATURE_STEP 0.0000001 // 0.0000001 or 0.0000005
// f(x) = -TEMPERATURE_STEP * x + STARTING_TEMPERATURE

#define CHANGE_DESCRIPTOR_EFFECT_CHANCE 0.8
#define CHANGE_COUNTY_MEMBERSHIP_CHANCE 1.0 - CHANGE_DESCRIPTOR_EFFECT_CHANCE

#define ADD_DESCRIPTOR_TO_COUNTY_CHANCE 0.5
#define REMOVE_DESCRIPTOR_FROM_COUNTY_CHANCE 1.0 - ADD_DESCRIPTOR_TO_COUNTY_CHANCE

#define ADD_REMOVE_BORDER_COUNTY_CHANCE 0.9
#define ADD_REMOVE_RANDOM_COUNTY_CHANCE 1.0 - ADD_REMOVE_BORDER_COUNTY_CHANCE

#define MIN_THREADS 1u
#define MAX_DEBUG_THREADS 1u
#define MAX_THREADS 100u

#define EXPECTED_NEIGHBORS_PER_COUNTY 7

// Score weights
#define ACCURACY_SCORE_WEIGHT    0.50 //0.500
#define SPECIFICITY_SCORE_WEIGHT 0.0 //0.100
#define PARSIMONY_SCORE_WEIGHT   0.50 //0.025
#define LOCALITY_SCORE_WEIGHT    0.0 //0.375

// Number of iterations between each time a thread will print its status (iteration count, temperature, accuracy)
#define PRINT_TSTATUS_EVERY 10000

#endif