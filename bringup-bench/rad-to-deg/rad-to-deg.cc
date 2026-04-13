#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"

fp64e_t 
rad2deg(fp64e_t rad)
{
      return ((fp64e_t)180.0 * rad / (M_PI));
}

fp64e_t
deg2rad(fp64e_t deg)
{
      return ((fp64e_t)M_PI * deg / 180.0);
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

  {
    // Stopwatch s("VIP_Bench Runtime");

    for (double x = 0.0; x <= 360.0; x += 1.0)
      libmin_printf("INFO: deg2rad(%.5lf) == %.5lf\n", x, deg2rad((fp64e_t)x).decrypt());

    for (double x = 0.0; x <= (2 * M_PI + 1e-6); x += (M_PI / 180))
      libmin_printf("INFO: rad2deg(%.5lf) == %.5lf\n", x, rad2deg((fp64e_t)x).decrypt());
  }

  libmin_success();
  return 0;
}
