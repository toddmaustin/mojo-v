#ifndef _PARAMETERS_CPP
#define _PARAMETERS_CPP

#include "parameters.h"

#include <iostream>
using namespace std;


SymmCipher CIPHER;
bit128_t SECRET_KEY;  

//Forward declaration of INTERFACE functions
void aes128_set_key(bit128_t key); 
void initialize_rng(int seed); 
void initialize_mersenne_rng(int seed);

void setParameters(){
        CIPHER = XOR; 
        SECRET_KEY.init(0, 5);
        initialize_mersenne_rng(1);
}

void setParameters(SymmCipher cipher){
        CIPHER = cipher; 
        SECRET_KEY.init(0, 5);
        initialize_mersenne_rng(1);

        switch(CIPHER){
            case AES128:    aes128_set_key(SECRET_KEY); break;
            default:        break;
        }
}

void setParameters(SymmCipher cipher, uint64_t key_upper, uint64_t key_lower, int seed){
        CIPHER = cipher; 
        SECRET_KEY.init(key_upper, key_lower);          
        initialize_mersenne_rng(seed);

        switch(CIPHER){
            case AES128:    aes128_set_key(SECRET_KEY); break;
            default:        break;
        }
}

void setParameters_XOR(uint64_t key_upper, uint64_t key_lower, int seed){
        CIPHER = SymmCipher::XOR; 
        SECRET_KEY.init(key_upper, key_lower);          
        initialize_mersenne_rng(seed);
}


#endif
