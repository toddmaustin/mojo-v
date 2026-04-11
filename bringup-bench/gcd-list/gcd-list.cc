#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"

/** Compute GCD using division algorithm
 *
 * @param[in] a array of integers to compute GCD for
 * @param[in] n number of integers in array `a`
 */
uint64e_t
gcd(uint64e_t *a, unsigned n)
{
  unsigned j = 1;  // to access all elements of the array starting from 1
  uint64e_t gcd = a[0];
  while (j < n)
  {
#define MAXITER 32  // any division >= 2 will reduce precision by at least 1 bit
    uint64e_t _done = false;
    for (unsigned iter=0; iter < MAXITER; iter++)
    {
      _done = !_done || (a[j] % gcd == 0);  // value of gcd is as needed so far
      gcd = cmov(_done, gcd, a[j] % gcd);  // calculating GCD by division method
    }
    j++;              // so we check for next element
  }
  return gcd;
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

  unsigned n = 64;
  uint64e_t *a = (uint64e_t *)libmin_malloc(n * sizeof(uint64e_t));
  for (unsigned i = 0; i < n; i++)
    a[i] = (libmin_rand() % 10000000) * 37;

  libmin_printf("INFO: a[%d] = { ", n);
  for (unsigned i = 0; i < n; i++)
    libmin_printf("%d, ", a[i].decrypt());
  libmin_printf(" }\n");

  uint64e_t gcd_of_n;
  {
    // Stopwatch s("VIP-Bench gcd-list:");

    gcd_of_n = gcd(a, n);
  }
  libmin_printf("GCD of list == %lu\n", gcd_of_n.decrypt());

  libmin_free(a);

  libmin_success();
  return 0;
}
