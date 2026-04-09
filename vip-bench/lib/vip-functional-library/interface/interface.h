#ifndef _INTERFACE_H
#define _INTERFACE_H

#include "parameters.h"


bit128_t encrypt_64_128(uint64_t* value);

uint64_t decrypt_128_64(bit128_t ciphertext);


bit128_t encrypt_8_128(uint8_t* value);

uint8_t decrypt_128_8(bit128_t ciphertext);


#endif
