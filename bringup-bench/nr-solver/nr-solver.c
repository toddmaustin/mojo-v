#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"
#include "dc-fast.h"
typedef mojov_mem_fast_u64_t _uint64e_t;
typedef mojov_mem_fast_fp64_t _fp64e_t;
#include "mojov-exo.h"

#define SECRET

#define MAXERR 0.00001
#define MAXITER 20

uint128_t simon_key = SIMON128_KEY;
simon_state_t simon_state;

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

// Run the fixed-iteration secret Newton-Raphson solver and return the encrypted root.
static _fp64e_t
nr_solver(_uint64e_t *converged)
{
  _fp64e_t guess = _fenc(1.0);
  _fp64e_t sqrt_secret = _fenc(sqrt_value);

  *converged = _enc(0);

  for (unsigned iter = 0; iter < MAXITER; ++iter)
  {
    _fp64e_t f_value = _fsub(_fmul(guess, guess), sqrt_secret);
    _fp64e_t abs_f_value = _fabs(f_value);
    _fp64e_t updated_guess = _fsub(guess, _fdiv(f_value, _fmuli(guess, 2.0)));

    *converged = _fslei(abs_f_value, MAXERR);
    guess = _fcmov(*converged, guess, updated_guess);
  }

  return guess;
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

  for (unsigned i = 0; i < NTESTDATA; ++i)
  {
    _uint64e_t converged;
    sqrt_value = testdata[i];
    _fp64e_t root_ct = nr_solver(&converged);
    double root = mojov_decrypt_fast_fp64(&simon_state, root_ct, CONTRACT_SIG);
    libmin_printf("sqrt(%lf) == %lf (converged:%c)\n",
      sqrt_value,
      root,
      mojov_decrypt_fast_u64(&simon_state, converged, CONTRACT_SIG) ? 't' : 'f');
  }

  libmin_success();
  return 0;
}
