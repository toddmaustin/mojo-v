#ifndef _PARAMETERS_H
#define _PARAMETERS_H
#include <stdlib.h> 
#include <random>
#include <bitset>
#include <string.h>
#include "bit_t.h"

enum SymmCipher {AES128, XOR};

// Encryption Scheme Parameters
extern SymmCipher CIPHER;
extern bit128_t SECRET_KEY;

void setParameters();
void setParameters(SymmCipher cipher);
void setParameters(SymmCipher cipher, uint64_t key_upper, uint64_t key_lower, int seed);
void setParameters_XOR(uint64_t key_upper, uint64_t key_lower, int seed);


#endif
