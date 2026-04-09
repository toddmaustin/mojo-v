#include <stdio.h>
#include <stdlib.h>

#include "../config.h"

void
encipher(VIP_ENCUINT *in, VIP_ENCUINT *out, VIP_ENCUINT *key)
{
  VIP_ENCUINT y=in[0], z=in[1], sum=0, delta=0x9E3779B9;
  VIP_ENCUINT a=key[0], b=key[1], c=key[2], d=key[3];
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
decipher(VIP_ENCUINT *in, VIP_ENCUINT *out, VIP_ENCUINT *key)
{
  VIP_ENCUINT y=in[0], z=in[1], sum=0xC6EF3720, delta=0x9E3779B9;
  VIP_ENCUINT a=key[0], b=key[1], c=key[2], d=key[3];
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


VIP_ENCUINT keytext[4];
VIP_ENCUINT plaintext[2];
VIP_ENCUINT ciphertext[2];
VIP_ENCUINT newplain[2];

int
main(void)
{
  VIP_INIT;
  unsigned int _keytext[4] = { 358852050,	311606025, 739108171, 861449956 };
  unsigned int _plaintext[2] = { 765625614, 14247501 };
  unsigned int cipherref[2] = { 0x9fe2c864, 0xd7da4da4 };

  // encrypt test inputs
  for (int i=0; i < 4; i++)
    keytext[i] = _keytext[i];
  for (int i=0; i < 2; i++)
    plaintext[i] = _plaintext[i];

  {
    Stopwatch s("VIP_Bench Runtime");

    encipher(plaintext, ciphertext, keytext);
    if (VIP_DEC(ciphertext[0]) != cipherref[0] || VIP_DEC(ciphertext[1]) != cipherref[1])
      return 1;
    decipher(ciphertext, newplain, keytext);
    if (VIP_DEC(newplain[0]) != _plaintext[0] || VIP_DEC(newplain[1]) != _plaintext[1])
      return 1;
  }
  
  printf("TEA Cipher results:\n");
  printf("  plaintext:  0x%08x 0x%08x\n", VIP_DEC(plaintext[0]), VIP_DEC(plaintext[1]));
  printf("  ciphertext: 0x%08x 0x%08x\n", VIP_DEC(ciphertext[0]), VIP_DEC(ciphertext[1]));
  printf("  newplain:   0x%08x 0x%08x\n", VIP_DEC(newplain[0]), VIP_DEC(newplain[1]));

  return 0;
}

