#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"

#define MAXERR 0.00001
#define MAXITER 20

static double sqrt_value;

static const double testdata[] = {
  395856.76220473,270306.80574294,932459.74833807,881022.81949615,70473.028447684,582103.8084143,37192.131696927,
  607938.62471086,329081.89022736,78531.037513184,325073.20247627,648692.01186933,744982.1258859,236439.23130461,
  7397.74047289,973218.15857982,846514.65828256,804528.86250616,804592.95559602,317002.2601471,539128.20585759,
  300805.543927,808726.5000455196,398639.79574811,746867.29179032,726986.58426265,89910.107895278,152448.39631835,
  971033.76322222,849626.4469692,834030.4248274,247231.09454278,863139.85370489,517243.77285195,798550.73169246,
  355200.70467728,10331.048309033,305804.09107807,958121.83380634,100665.02513818,540398.10389697,361497.4429536,
  571322.02067934,219532.94205547,3315.861315617,290434384.19881872,479817.47545507,307333.29886535,71797.849784014,
  870080.84816375
};
#define NTESTDATA (sizeof(testdata) / sizeof(testdata[0]))

// Run the fixed-iteration secret Newton-Raphson
// solver and return the encrypted root.
static fp64e_t
nr_solver(uint64e_t *converged)
{
  fp64e_t guess = 1.0;
  fp64e_t sqrt_secret = sqrt_value;

  *converged = 0;

  for (unsigned iter = 0; iter < MAXITER; ++iter)
  {
    fp64e_t f_value = (guess * guess) - sqrt_secret;
    fp64e_t updated_guess = guess - f_value / (guess * 2.0);

    *converged = fabs(f_value) <= MAXERR;
    guess = _fcmov(*converged, guess, updated_guess);
  }

  return guess;
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

  for (unsigned i = 0; i < NTESTDATA; ++i)
  {
    uint64e_t converged;
    sqrt_value = testdata[i];
    fp64e_t root = nr_solver(&converged);
    libmin_printf("sqrt(%lf) == %lf (converged:%c)\n", sqrt_value, root.decrypt(), converged.decrypt() ? 't' : 'f');
  }

  libmin_success();
  return 0;
}
