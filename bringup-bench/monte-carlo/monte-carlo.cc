#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"

#define NUM_SAMPLES 25000 // samples

int
main(void)
{
  if (mojov_configure_kmsm_from_dc_fast() != 0)
    return -1;

  if (mojov_enable_and_verify() != 0)
    return -1;

  if (debug_context(SIMON128_KEY, CONTRACT_SIG) != 0)
    return -1;

  uint64e_t count_inside_circle = 0;
  fp64e_t x;
  fp64e_t y;

  // Seed the random number generator
  libmin_srand(42);

  for (int i = 0; i < NUM_SAMPLES; ++i)
  {
    // Generate random (x, y) point in [0, 1] × [0, 1]
    x = (double)libmin_rand() / RAND_MAX;
    y = (double)libmin_rand() / RAND_MAX;

    // Check if the point is inside the unit circle
    if (x * x + y * y <= 1.0)
      count_inside_circle = count_inside_circle + 1;
  }

  // Estimate Pi
  fp64e_t pi_estimate = 4.0 * count_inside_circle / NUM_SAMPLES;

  // Output result
  libmin_printf("Estimated Pi = %.8f\n", pi_estimate.decrypt());

  libmin_success();

  return 0;
}
