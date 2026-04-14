#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"

#include "nonlinear-nn-inputs.h"

// VIP type mapping for this port:
//   VIP_ENCCHAR   -> int8e_t
//   VIP_ENCBOOL   -> uint64e_t
//   VIP_ENCUINT64 -> uint64e_t
//   VIP_ENCFLOAT  -> fp32e_t
//   VIP_ENCDOUBLE -> fp64e_t

// LeakyReLU uses a small slope for negative values.
static const double kLeakyAlpha = 0.01;

// ReLU(x) = max(x, 0).
static inline fp64e_t
relu(fp64e_t x)
{
  return cmov(x > (fp64e_t)0.0, x, (fp64e_t)0.0);
}

// LeakyReLU(x) = x when x>0, else alpha*x.
static inline fp64e_t
leaky_relu(fp64e_t x, fp64e_t alpha)
{
  return cmov(x > (fp64e_t)0.0, x, alpha * x);
}

int
main(void)
{
  if (mojov_configure_kmsm_from_dc_fast() != 0)
    return -1;

  // Enable private register semantics (bit 0 = 1).
  if (mojov_enable_and_verify() != 0)
    return -1;

  // Enable encrypted variable debugging.
  if (debug_context(SIMON128_KEY, CONTRACT_SIG) != 0)
    return -1;

  // Initialize the pseudo-RNG.
  libmin_srand(42);

  // Encrypted globals cannot currently be initialized statically in Mojo-V,
  // so populate encrypted arrays and constants explicitly in main().
  fp64e_t alpha = (fp64e_t)kLeakyAlpha;
  fp64e_t inputs[NONLINEAR_NN_INPUT_COUNT];
  for (unsigned i = 0; i < NONLINEAR_NN_INPUT_COUNT; i++)
    inputs[i] = (fp64e_t)nonlinear_nn_inputs_plain[i];

  for (unsigned i = 0; i < NONLINEAR_NN_INPUT_COUNT; i++)
  {
    fp64e_t x = inputs[i];
    libmin_printf("INFO: ReLU(%.5lf) == %.5lf\n",
                  nonlinear_nn_inputs_plain[i],
                  relu(x).decrypt());
    libmin_printf("INFO: LeakyReLU(%.5lf) == %.5lf\n",
                  nonlinear_nn_inputs_plain[i],
                  leaky_relu(x, alpha).decrypt());
  }

  libmin_success();
  return 0;
}
