#ifndef CONSTANTS_H
#define CONSTANTS_H

// The number of demographic keys used in the dataset
#define NUMBER_DEMOGRAPHICS 134
// The number of descriptors we will create (including fixed-membership [USA, AL, etc])
#define NUMBER_DESCRIPTORS 500
// Maximum change (positive or negative) to a descriptor effect from one permutation - percentage
#define MAX_CHANGE_AMT 0.25

#define MAX_ITERATIONS 100'000'000
#define MAX_TRIES 500000

// Temperature is the probability that a worse change is accepted
#define STARTING_TEMPERATURE 1.0
#define TEMPERATURE_STEP 0.0000002

#define CHANGE_DESCRIPTOR_CHANCE 0.99
#define CHANGE_COUNTY_CHANCE 1.0 - CHANGE_DESCRIPTOR_CHANCE

#define MIN_THREADS 1u
#define MAX_THREADS 100u

#define EXPECTED_NEIGHBORS_PER_COUNTY 7

#define ACCURACY_SCORE_WEIGHT    0.75
#define SPECIFICITY_SCORE_WEIGHT 0.15
#define PARSIMONY_SCORE_WEIGHT   0.08
#define LOCALITY_SCORE_WEIGHT    0.02

// Number of iterations between each time a thread will print its status (iteration count, temperature, accuracy)
#define PRINT_TSTATUS_EVERY 10000

#endif