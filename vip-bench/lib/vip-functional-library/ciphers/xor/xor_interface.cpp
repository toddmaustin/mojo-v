#include "xor_interface.h"



bit128_t xor_encrypt_128(bit128_t plaintext){
    bit128_t ciphertext;
    for(int i=0; i<16; i++){
        ciphertext.value[i] = plaintext.value[i]^SECRET_KEY.value[i];
    }
    return ciphertext;
}

bit128_t xor_decrypt_128(bit128_t ciphertext){
    bit128_t plaintext;
    for(int i=0; i<16; i++){
        plaintext.value[i] = ciphertext.value[i]^SECRET_KEY.value[i];
    }
    return plaintext;
}


