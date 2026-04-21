#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"

#define LIMIT 8
#define TRUE 1
#define FALSE 0

static char flags[8192];

typedef struct
{
  uint64e_t n_prime;
  uint64e_t l_prime;
} sieve_result_t;

static sieve_result_t
SIEVE(uint64e_t m, uint64e_t p)
{
  long i, k;
  uint64e_t prime = 0;
  uint64e_t count = 0;
  long size;
  sieve_result_t result;

  size = (long)m.decrypt() - 1;
  result.n_prime = 0ul;
  result.l_prime = 0ul;

  for (i = 0; i <= size; i++)
    flags[i] = TRUE;

  for (i = 0; i <= size; i++)
  {
    if (flags[i])
    {
      count = count + 1;
      prime = (uint64_t)(i + i + 3);

      for (k = i + (long)prime.decrypt(); k <= size; k += (long)prime.decrypt())
        flags[k] = FALSE;
    }
  }

  result.n_prime = count;
  result.l_prime = prime;
  (void)p;
  return result;
}

int
main(void)
{
  if (mojov_configure_kmsm_from_dc_fast() != 0)
    return -1;

  if (mojov_enable_and_verify() != 0)
    return -1;

  if (debug_context(SIMON128_KEY, CONTRACT_SIG) != 0)
    return -1;

  uint64e_t j = 1024;
  uint64e_t p = 1;

  libmin_printf("\n Sieve of Eratosthenes (Scaled to 10 Iterations)\n");
  libmin_printf(" Version 1.2b, 26 Sep 1992\n\n");
  libmin_printf(" Array Size Number Last Prime\n");
  libmin_printf(" (Bytes) of Primes\n");

  sieve_result_t result = SIEVE(j, p);
  if (p.decrypt() != 0L)
    libmin_printf(" %9ld %8ld %8ld\n", j.decrypt(), result.n_prime.decrypt(), result.l_prime.decrypt());

  libmin_success();
  return 0;
}
