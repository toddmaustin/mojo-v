#ifndef _RNG_INTERFACE_H
#define _RNG_INTERFACE_H

#include "../interface/parameters.h"
#include "mersenne-twister-1/mersenne-twister.h"
#include <iostream>
using namespace std;




void initialize_rng(int);

uint64_t get_random_pad_64b();

uint8_t get_random_pad_8b();

// Random number generator using the Mersenne-Twister library
void initialize_mersenne_rng(int);

uint64_t get_mersenne_random_pad_64b();

uint64_t get_mersenne_random_pad_8b();

#endif
