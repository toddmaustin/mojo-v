#ifndef _AES_INTERFACE_H
#define _AES_INTERFACE_H

#include "../../interface/parameters.h"
#include "aes128ni.h"


#define NUM_OF_NIST_KEYS 16
#define NUM_OF_FIXED_KEYS 128



void aes128_set_key(bit128_t);

bit128_t aes128_encrypt_128(bit128_t);

bit128_t aes128_decrypt_128(bit128_t);

#endif
