#ifndef _AES_INTERFACE_CPP
#define _AES_INTERFACE_CPP

#include "aes_interface.h"


struct aes128 ctx[1];

void aes128_set_key(bit128_t key_pass){
    aes128_init(ctx, key_pass.value);
}


bit128_t aes128_encrypt_128(bit128_t plaintext){
	unsigned char ct[AES128_BLOCKLEN] = {0};
	aes128_encrypt(ctx, ct, plaintext.value);

    return bit128_t(ct);
}


bit128_t aes128_decrypt_128(bit128_t ciphertext){

	unsigned char pt[AES128_BLOCKLEN] = {0};
	aes128_decrypt(ctx, pt, ciphertext.value);

    return bit128_t(pt);
}


#endif
