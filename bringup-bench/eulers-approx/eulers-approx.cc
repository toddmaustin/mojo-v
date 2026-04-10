#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"

#define M_E 2.71828182845904523536
#define MAX_STEPS 10000000l

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

  uint64_t steps = MAX_STEPS; /* STEPS is usually a very large number eg 10000000 */
  fp64e_t e, term;

  // comupute "e" via compound interest limit
  e = 1.0;
  term = 1.0 + (1.0/steps);

  for(; steps > 0; steps--)
    e = e * term;

  libmin_printf("INFO: e via compound interest after %lu steps == %.16lf (%.16lf error).\n",
                MAX_STEPS, e.decrypt(), M_E - e.decrypt());

  // compute "e" via the infinite series method
  e = 1.0;              // Starts at 1/0! = 1
  term = 1.0;   // The current fraction being added (initially 1/1!)
  int k = 1;
  uint64e_t cnt = 0;

  // keep adding terms until the fractions become smaller than 
  // what a double can accurately hold.
  for (k=1; k <= 64; k++)
  {
    int64e_t pred = (term > 1e-17);
    term = cmov(pred, term / k, term); // Generates the next term: 1/k!
    e = cmov(pred, e + term, e); // Adds 1/k! to the total
    cnt = cmov(pred, cnt + 1, cnt);
  }

  // print the result with maximum double precision
  libmin_printf("INFO: e via infinite series after %u steps == %.16lf (%.16lf error).\n",
                cnt.decrypt(), e.decrypt(), M_E - e.decrypt());

  libmin_success();
  return 0;
}
