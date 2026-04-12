#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"

void
encipher(uint32e_t *in, uint32e_t *out, uint32e_t *key)
{
  uint32e_t y=in[0], z=in[1], sum=(uint32e_t)0;
  uint32e_t delta=(uint32_t)0x9e3779b9u;
  uint32e_t a=key[0], b=key[1], c=key[2], d=key[3];
  unsigned int n=32;

  while (n-->0)
  {
    sum = sum + delta;
    y = y + (((z << 4)+a) ^ (z+sum) ^ ((z >> 5)+b));
    z = z + (((y << 4)+c) ^ (y+sum) ^ ((y >> 5)+d));
  }
  out[0]=y; out[1]=z;
}

void
decipher(uint32e_t *in, uint32e_t *out, uint32e_t *key)
{
  uint32e_t y=in[0], z=in[1], sum=(uint32e_t)0xc6ef3720u, delta=(uint32e_t)0x9e3779b9u;
  uint32e_t a=key[0], b=key[1], c=key[2], d=key[3];
  unsigned int n=32;

  /* sum = delta<<5, in general sum = delta * n */
  while (n-->0)
  {
    z = z - (((y << 4)+c) ^ (y+sum) ^ ((y >> 5)+d));
    y = y - (((z << 4)+a) ^ (z+sum) ^ ((z >> 5)+b));
    sum = sum - delta;
  }
  out[0]=y; out[1]=z;
}


int
main(void)
{
  if (mojov_configure_kmsm_from_dc_fast() != 0)
    return -1;

  // enable private register semantics (bit 0 = 1)
  if (mojov_enable_and_verify() != 0)
    return -1;

  // enable encrypted variable debugging
  if (debug_context(SIMON128_KEY, CONTRACT_SIG) != 0)
    return -1;

  // initialize the pseudo-RNG
  libmin_srand(42);

  uint32e_t keytext[4];
  uint32_t _keytext[4] = { 358852050u,	311606025u, 739108171u, 861449956u };
  uint32e_t plaintext[2];
  uint32_t _plaintext[2] = { 765625614u, 14247501u };

  uint32e_t newplain[2];
  uint32e_t ciphertext[2];
  uint32_t cipherref[2] = { 0x9fe2c864u, 0xd7da4da4u };

  // encrypt test inputs
  for (int i=0; i < 4; i++)
    keytext[i] = _keytext[i];
  for (int i=0; i < 2; i++)
    plaintext[i] = _plaintext[i];

  {
    // Stopwatch s("VIP_Bench Runtime");

    encipher(plaintext, ciphertext, keytext);
    if (ciphertext[0].decrypt() != cipherref[0] || ciphertext[1].decrypt() != cipherref[1])
    {
      libmin_printf("ERROR: invalid encryption\n");
      return 1;
    }
    decipher(ciphertext, newplain, keytext);
    if (newplain[0].decrypt() != _plaintext[0] || newplain[1].decrypt() != _plaintext[1])
    {
      libmin_printf("ERROR: invalid decryption\n");
      return 1;
    }
  }
  
  libmin_printf("TEA Cipher results:\n");
  libmin_printf("  plaintext:  0x%08x 0x%08x\n", plaintext[0].decrypt(), plaintext[1].decrypt());
  libmin_printf("  ciphertext: 0x%08x 0x%08x\n", ciphertext[0].decrypt(), ciphertext[1].decrypt());
  libmin_printf("  newplain:   0x%08x 0x%08x\n", newplain[0].decrypt(), newplain[1].decrypt());

  libmin_success();
  return 0;
}
