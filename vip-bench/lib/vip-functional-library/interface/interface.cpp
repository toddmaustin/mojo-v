#ifndef _INTERFACE_CPP
#define _INTERFACE_CPP
#include "interface.h"
#include "../ciphers/aes/aes_interface.h"
#include "../ciphers/xor/xor_interface.h"
#include "../rng/rng_interface.h"
#include "parameters.h"



bit128_t encrypt_64_128(uint64_t* value){
    uint64_t pad = get_mersenne_random_pad_64b();
    bit128_t plaintext(*value, pad);

    switch(CIPHER){
        case XOR:       return xor_encrypt_128(plaintext);
        case AES128:    return aes128_encrypt_128(plaintext);
        default:        abort();
    }
}

uint64_t decrypt_128_64(bit128_t ciphertext){
    switch(CIPHER){
        case XOR:       return xor_decrypt_128(ciphertext).getUpperValue_64b();
        case AES128:    return aes128_decrypt_128(ciphertext).getUpperValue_64b();
        default:        abort();
    }
}

bit128_t encrypt_8_128(uint8_t* value){
    uint8_t pad[15];
    for(int i=0; i<15; i++){
        pad[i] = get_mersenne_random_pad_8b();
    }
    bit128_t plaintext(*value, pad);

    switch(CIPHER){
        case XOR:       return xor_encrypt_128(plaintext);
        case AES128:    return aes128_encrypt_128(plaintext);
        default:        abort();
    }
}

uint8_t decrypt_128_8(bit128_t ciphertext){
    switch(CIPHER){
        case XOR:       return xor_decrypt_128(ciphertext).getUpperValue_8b();
        case AES128:    return aes128_decrypt_128(ciphertext).getUpperValue_8b();
        default:        abort();
    }
}


#endif
