#ifndef SIMON_H
#define SIMON_H

#include <stdint.h>
typedef unsigned __int128 uint128_t;
#define HI64(X)        ((uint64_t)(((X) >> 64) & 0xffffffffffffffffULL))
#define LO64(X)        ((uint64_t)(((X))       & 0xffffffffffffffffULL))
#define GEN128(HI, LO) ((((uint128_t)(HI)) << 64) | ((uint128_t)(LO)))

#include "simon_internal.h"

// project interfaces to the Simon software core
typedef struct {
  SimSpk_Cipher x;
} simon_state_t;


/* Simon cipher: 64-bit key, 32-bit data */
bool simon_64_32_keyexpand(simon_state_t *simon_state, uint64_t key, unsigned round_limit = /* full strength */32);
void simon_64_32_encrypt(simon_state_t *simon_state, uint32_t plaintext, uint32_t *ciphertext);
void simon_64_32_decrypt(simon_state_t *simon_state, uint32_t ciphertext, uint32_t *plaintext);

/* Simon cipher: 128-bit key, 64-bit data */
bool simon_128_64_keyexpand(simon_state_t *simon_state, uint128_t key, unsigned round_limit = /* full strength */44);
void simon_128_64_encrypt(simon_state_t *simon_state, uint64_t plaintext, uint64_t *ciphertext);
void simon_128_64_decrypt(simon_state_t *simon_state, uint64_t ciphertext, uint64_t *plaintext);

/* Simon cipher: 128-bit key, 128-bit data */
bool simon_128_128_keyexpand(simon_state_t *simon_state, uint128_t key, unsigned round_limit = /* full strength */68);
void simon_128_128_encrypt(simon_state_t *simon_state, uint128_t plaintext, uint128_t *ciphertext);
void simon_128_128_decrypt(simon_state_t *simon_state, uint128_t ciphertext, uint128_t *plaintext);

// hook functions to the validated Simon cipher core

inline extern bool
simon_64_32_keyexpand(simon_state_t *simon_state, uint64_t key, unsigned round_limit /* full strength = 32 */)
{
  uint8_t result, iv = 0, counter = 0;

  result = Simon_Init(&simon_state->x, cfg_64_32, ECB, &key, &iv, &counter);
  simon_state->x.round_limit = round_limit;
  return (result == 0);
}

inline extern void
simon_64_32_encrypt(simon_state_t *simon_state, uint32_t plaintext, uint32_t *ciphertext)
{
  Simon_Encrypt_32(simon_state->x.round_limit, simon_state->x.key_schedule, (const uint8_t *)&plaintext, (uint8_t *)ciphertext);
}

inline extern void
simon_64_32_decrypt(simon_state_t *simon_state, uint32_t ciphertext, uint32_t *plaintext)
{
  Simon_Decrypt_32(simon_state->x.round_limit, simon_state->x.key_schedule, (const uint8_t *)&ciphertext, (uint8_t *)plaintext);
}

inline extern bool
simon_128_64_keyexpand(simon_state_t *simon_state, uint128_t key, unsigned round_limit /* full strength = 44 */)
{
  uint8_t result, iv = 0, counter = 0;

  result = Simon_Init(&simon_state->x, cfg_128_64, ECB, &key, &iv, &counter);
  simon_state->x.round_limit = round_limit;
  return (result == 0);
}

inline extern void
simon_128_64_encrypt(simon_state_t *simon_state, uint64_t plaintext, uint64_t *ciphertext)
{
  Simon_Encrypt_64(simon_state->x.round_limit, simon_state->x.key_schedule, (const uint8_t *)&plaintext, (uint8_t *)ciphertext);
}

inline extern void
simon_128_64_decrypt(simon_state_t *simon_state, uint64_t ciphertext, uint64_t *plaintext)
{
  Simon_Decrypt_64(simon_state->x.round_limit, simon_state->x.key_schedule, (const uint8_t *)&ciphertext, (uint8_t *)plaintext);
}

inline extern bool
simon_128_128_keyexpand(simon_state_t *simon_state, uint128_t key, unsigned round_limit /* full strength = 44 */)
{
  uint8_t result, iv = 0, counter = 0;

  result = Simon_Init(&simon_state->x, cfg_128_128, ECB, &key, &iv, &counter);
  simon_state->x.round_limit = round_limit;
  return (result == 0);
}

inline extern void
simon_128_128_encrypt(simon_state_t *simon_state, uint128_t plaintext, uint128_t *ciphertext)
{
  Simon_Encrypt_128(simon_state->x.round_limit, simon_state->x.key_schedule, (const uint8_t *)&plaintext, (uint8_t *)ciphertext);
}

inline extern void
simon_128_128_decrypt(simon_state_t *simon_state, uint128_t ciphertext, uint128_t *plaintext)
{
  Simon_Decrypt_128(simon_state->x.round_limit, simon_state->x.key_schedule, (const uint8_t *)&ciphertext, (uint8_t *)plaintext);
}

#endif /* SIMON_H */

