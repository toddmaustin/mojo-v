#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

typedef mojov_mem_fast_u64_t _uint64e_t;
typedef mojov_mem_fast_fp64_t _fp64e_t;
#include "mojov-exo.h"

#define K 16ull
#define PT_COMPOSITE 0ul
#define PT_PRIME 1ul
#define PT_PRIME_LIKELY 2ul
#define Q_SIZE NTESTS
#define NTESTS 200

struct primality_result {
  uint64e_t val;
  uint64e_t prim;
};

static struct primality_result q[Q_SIZE];
static uint32_t q_head;

static uint64_t
get_random_int(uint64_t low, uint64_t high)
{
  return (uint64_t)libmin_rand() % (high - low + 1) + low;
}

static void
split_int(uint64_t *s, uint64_t *d, uint64_t n)
{
  *s = 0;
  *d = n - 1;

  while ((*d & 1ull) == 0)
  {
    (*s)++;
    *d >>= 1;
  }
}

static uint64e_t
powm_secret(uint64e_t base, uint64_t exponent, uint64e_t modulus)
{
  uint64e_t result = 1;

  while (exponent != 0)
  {
    if ((exponent & 1ull) != 0)
      result = (result * base) % modulus;
    exponent >>= 1;
    if (exponent != 0)
      base = (base * base) % modulus;
  }

  return result;
}

static uint64e_t
miller_rabin_secret(uint64e_t n_enc, uint64_t n)
{
  uint64e_t prim_secret;
#if 0
  _store(&prim_secret, PT_COMPOSITE);

  uint64e_t done;
  _store(&done, 0);

  uint64e_t pred = _seqi(n_enc, 2u);
  pred = _seqi(_andi(n_enc, 1u), 0u);
#endif
 
  if ((n & 1u) == 0)
  {
    prim_secret = n == 2u ? PT_PRIME : PT_COMPOSITE;
    return prim_secret;
  }
  if (n == 3u)
  {
    return PT_PRIME;
  }
  if (n < 3u)
  {
    return PT_COMPOSITE;
  }

  uint64e_t n_secret = n;
  uint64e_t nm1_secret = (uint64e_t)((uint64_t)n - 1);
  uint64e_t composite_secret = 0;

  uint64_t s;
  uint64_t d;
  split_int(&s, &d, n);

  for (uint32_t i = 0; i < K; ++i)
  {
    const uint64_t a_public = get_random_int(2, (uint64_t)n - 2ull);
    uint64e_t a_secret = a_public;
    uint64e_t witness_composite = 0;
    uint64e_t passed_witness;

    uint64e_t x = powm_secret(a_secret, d, n_secret);
    passed_witness = (x == 1) || (x == nm1_secret);

    for (uint64_t r = 1; r <= s; ++r)
    {
      x = (x * x) % n_secret;
      uint64e_t hit_one = (x == 1);
      uint64e_t hit_nm1 = (x == nm1_secret);
      witness_composite = (witness_composite || (!passed_witness && hit_one));
      passed_witness = (passed_witness || hit_nm1);
    }

    witness_composite = (witness_composite || !passed_witness);
    composite_secret = (composite_secret || witness_composite);
  }

  return cmov(composite_secret, PT_COMPOSITE, PT_PRIME_LIKELY);
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

  libmin_srand(42);

  uint64_t val = 3;
  for (uint32_t i = 0; i < NTESTS; ++i)
  {
    q[i].val = val;
    q[i].prim = miller_rabin_secret(q[i].val, val);
    q_head++;
    val = (uint32_t)libmin_rand();
  }

  uint32_t prime_count = 0;
  for (uint32_t i = 0; i < q_head; ++i)
  {
    if (q[i].prim.decrypt() != PT_COMPOSITE)
      prime_count++;
  }

  libmin_printf("Primality tests found %u primes...\n", prime_count);
  for (uint32_t i = 0; i < q_head; ++i)
  {
    if (q[i].prim.decrypt() == PT_PRIME)
      libmin_printf("Value %lu is `prime' with failure probability (0)\n", q[i].val.decrypt());
    else if (q[i].prim.decrypt() == PT_PRIME_LIKELY)
      libmin_printf("Value %lu is `likely prime' with failure probability (1 in %lu)\n",
                    q[i].val.decrypt(), 4ull*4ull*4ull*4ull*4ull*4ull*4ull*4ull*4ull*4ull*4ull*4ull*4ull*4ull*4ull*4ull);
  }

  libmin_success();
  return 0;
}
