#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"
#include "dc-fast.h"
typedef mojov_mem_fast_u64_t _uint64e_t;
typedef mojov_mem_fast_fp64_t _fp64e_t;
#include "mojov-exo.h"

#define K 16u
#define PT_COMPOSITE 0u
#define PT_PRIME 1u
#define PT_PRIME_LIKELY 2u
#define Q_SIZE NTESTS
#define NTESTS 200

uint128_t simon_key = SIMON128_KEY;
simon_state_t simon_state;

struct primality_result {
  _uint64e_t val;
  _uint64e_t prim;
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

static _uint64e_t
powm_secret(_uint64e_t base, uint64_t exponent, _uint64e_t modulus)
{
  _uint64e_t result = _enc(1);

  while (exponent != 0)
  {
    if ((exponent & 1ull) != 0)
      result = _mod(_mul(result, base), modulus);
    exponent >>= 1;
    if (exponent != 0)
      base = _mod(_mul(base, base), modulus);
  }

  return result;
}

static _uint64e_t
miller_rabin_secret(_uint64e_t n_enc, uint64_t n)
{
  _uint64e_t prim_secret;
#if 0
  _store(&prim_secret, PT_COMPOSITE);

  _uint64e_t done;
  _store(&done, 0);

  _uint64e_t pred = _seqi(n_enc, 2u);
  pred = _seqi(_andi(n_enc, 1u), 0u);
#endif
 
  if ((n & 1u) == 0)
  {
    prim_secret = _enc(n == 2u ? PT_PRIME : PT_COMPOSITE);
    return prim_secret;
  }
  if (n == 3u)
  {
    return _enc(PT_PRIME);
  }
  if (n < 3u)
  {
    return _enc(PT_COMPOSITE);
  }

  _uint64e_t n_secret = _enc(n);
  _uint64e_t nm1_secret = _enc((uint64_t)n - 1ull);
  _uint64e_t composite_secret = _enc(0);

  uint64_t s;
  uint64_t d;
  split_int(&s, &d, n);

  for (uint32_t i = 0; i < K; ++i)
  {
    const uint64_t a_public = get_random_int(2, (uint64_t)n - 2ull);
    _uint64e_t a_secret = _enc(a_public);
    _uint64e_t witness_composite = _enc(0);
    _uint64e_t passed_witness;

    _uint64e_t x = powm_secret(a_secret, d, n_secret);
    passed_witness = _lor(_seqi(x, 1), _seq(x, nm1_secret));

    for (uint64_t r = 1; r <= s; ++r)
    {
      x = _mod(_mul(x, x), n_secret);
      _uint64e_t hit_one = _seqi(x, 1);
      _uint64e_t hit_nm1 = _seq(x, nm1_secret);
      witness_composite = _lor(witness_composite, _land(_lnot(passed_witness), hit_one));
      passed_witness = _lor(passed_witness, hit_nm1);
    }

    witness_composite = _lor(witness_composite, _lnot(passed_witness));
    composite_secret = _lor(composite_secret, witness_composite);
  }

  prim_secret = _cmov(composite_secret, _enc(PT_COMPOSITE), _enc(PT_PRIME_LIKELY));
  return prim_secret;
}

int
main(void)
{
  if (mojov_configure_kmsm_from_dc_fast() != 0)
    return -1;

  simon_128_128_keyexpand(&simon_state, simon_key, 68);

  libmin_printf("** Running CSR[privreg] tests...\n");

  uint64_t val = mojov_read_mprivregcfg();
  libmin_printf("Initial mprivregcfg = 0x%lx, ", val);
  mojov_print_mprivregcfg(val);
  libmin_printf("\n");

  if (mojov_enable_and_verify() != 0)
    return -1;

  val = mojov_read_mprivregcfg();
  libmin_printf("After enable, mprivregcfg = 0x%lx, ", val);
  mojov_print_mprivregcfg(val);
  libmin_printf("\n");

  libmin_srand(42);

  val = 3;
  for (uint32_t i = 0; i < NTESTS; ++i)
  {
    q[i].val = _enc(val);
    q[i].prim = miller_rabin_secret(q[i].val, val);
    q_head++;
    val = (uint32_t)libmin_rand();
  }

  uint32_t prime_count = 0;
  for (uint32_t i = 0; i < q_head; ++i)
  {
    uint64_t prim = mojov_decrypt_fast_u64(&simon_state, q[i].prim, CONTRACT_SIG);
    if (prim != PT_COMPOSITE)
      prime_count++;
  }

  libmin_printf("Primality tests found %u primes...\n", prime_count);
  for (uint32_t i = 0; i < q_head; ++i)
  {
    uint64_t val = mojov_decrypt_fast_u64(&simon_state, q[i].val, CONTRACT_SIG);
    uint64_t prim = mojov_decrypt_fast_u64(&simon_state, q[i].prim, CONTRACT_SIG);
    if (prim == PT_PRIME)
      libmin_printf("Value %lu is `prime' with failure probability (0)\n", val);
    else if (prim == PT_PRIME_LIKELY)
      libmin_printf("Value %lu is `likely prime' with failure probability (1 in %.0lf)\n", val, libmin_pow(4.0, K));
  }

  libmin_success();
  return 0;
}
