#ifndef CONSTANTS_H
#define CONSTANTS_H

// The number of demographic keys used in the dataset
#define NUMBER_DEMOGRAPHICS 134
// The number of descriptors we will create (including fixed-membership [USA, AL, etc])
#define NUMBER_DESCRIPTORS 500
// Maximum change (positive or negative) to a descriptor effect from one permutation - percentage
#define MAX_CHANGE_AMT 0.25

#define MAX_TRIES 500000

#define MIN_THREADS 1u
#define MAX_THREADS 100u

#endif