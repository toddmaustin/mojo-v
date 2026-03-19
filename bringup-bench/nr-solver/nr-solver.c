#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"
#include "dc-fast.h"

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

// Encrypt and store a public FP64 value into FAST-format secret memory.
static void
secret_store_fp64(double value, mojov_mem_fast_fp64_t *dst)
{
  __asm__ volatile (
    "fld   f28, (%0)\n\t"
    FSDE(  f28, %1, 0)
    :
    : "r" (&value), "r" (dst)
    : "f28", "memory"
  );
}

// Encrypt and store a public U64 value into FAST-format secret memory.
static void
secret_store_u64(uint64_t value, mojov_mem_fast_u64_t *dst)
{
  __asm__ volatile (
    "ld   x28, (%0)\n\t"
    SDE(  x28, %1, 0)
    :
    : "r" (&value), "r" (dst)
    : "x28", "memory"
  );
}

// Compute the secret Newton-Raphson residual x*x - sqrt_value.
static void
secret_f(double public_sqrt_value, mojov_mem_fast_fp64_t *x, mojov_mem_fast_fp64_t *dst)
{
  __asm__ volatile (
    FLDE(  f28, %1, 0)
    "fld   f29, (%0)\n\t"
    "fmul.d f30, f28, f28\n\t"
    "fsub.d f30, f30, f29\n\t"
    FSDE(  f30, %2, 0)
    :
    : "r" (&public_sqrt_value), "r" (x), "r" (dst)
    : "f28", "f29", "f30", "memory"
  );
}

// Compute the absolute value of a secret FP64 input with data-oblivious selection.
static void
secret_fabs(mojov_mem_fast_fp64_t *src, mojov_mem_fast_fp64_t *dst)
{
  __asm__ volatile (
    FLDE(  f28, %0, 0)
    "mv      x28, x0\n\t"
    "fmv.d.x f29, x28\n\t"
    "flt.d   x30, f28, f29\n\t"
    "fneg.d  f30, f28\n\t"
    "fmv.x.d x28, f28\n\t"
    "fmv.x.d x29, f30\n\t"
    "czero.eqz x31, x29, x30\n\t"
    "czero.nez x30, x28, x30\n\t"
    "or        x31, x30, x31\n\t"
    "fmv.d.x f31, x31\n\t"
    FSDE(      f31, %1, 0)
    :
    : "r" (src), "r" (dst)
    : "x28", "x29", "x30", "x31", "f28", "f29", "f30", "f31", "memory"
  );
}

// Compare a secret FP64 value against a public bound and store a secret boolean.
static void
secret_le_public(mojov_mem_fast_fp64_t *src, double rhs, mojov_mem_fast_u64_t *dst)
{
  __asm__ volatile (
    FLDE(  f28, %0, 0)
    "fld   f29, (%1)\n\t"
    "fle.d  x28, f28, f29\n\t"
    SDE(   x28, %2, 0)
    :
    : "r" (src), "r" (&rhs), "r" (dst)
    : "x28", "f28", "f29", "memory"
  );
}

// Compute one secret Newton-Raphson update step for the current guess.
static void
secret_nr_update(double public_sqrt_value, mojov_mem_fast_fp64_t *guess, mojov_mem_fast_fp64_t *dst)
{
  static const double two = 2.0;

  __asm__ volatile (
    FLDE(  f28, %1, 0)
    "fld   f29, (%0)\n\t"
    "fld   f30, (%2)\n\t"
    "fmul.d f31, f28, f28\n\t"
    "fsub.d f31, f31, f29\n\t"
    "fmul.d f30, f28, f30\n\t"
    "fdiv.d f31, f31, f30\n\t"
    "fsub.d f31, f28, f31\n\t"
    FSDE(  f31, %3, 0)
    :
    : "r" (&public_sqrt_value), "r" (guess), "r" (&two), "r" (dst)
    : "f28", "f29", "f30", "f31", "memory"
  );
}

// Select between two secret FP64 values using a secret predicate.
static void
secret_select_fp64(mojov_mem_fast_u64_t *predicate, mojov_mem_fast_fp64_t *if_true,
                   mojov_mem_fast_fp64_t *if_false, mojov_mem_fast_fp64_t *dst)
{
  __asm__ volatile (
    LDE(   x30, %0, 0)
    FLDE(  f28, %1, 0)
    FLDE(  f29, %2, 0)
    "fmv.x.d x28, f28\n\t"
    "fmv.x.d x29, f29\n\t"
    "czero.eqz x31, x28, x30\n\t"
    "czero.nez x30, x29, x30\n\t"
    "or        x31, x30, x31\n\t"
    "fmv.d.x f30, x31\n\t"
    FSDE(      f30, %3, 0)
    :
    : "r" (predicate), "r" (if_true), "r" (if_false), "r" (dst)
    : "x28", "x29", "x30", "x31", "f28", "f29", "f30", "memory"
  );
}

// Run the fixed-iteration secret Newton-Raphson solver and return the encrypted root.
static mojov_mem_fast_fp64_t
nr_solver(mojov_mem_fast_u64_t *converged)
{
  mojov_mem_fast_fp64_t guess;
  mojov_mem_fast_fp64_t f_value;
  mojov_mem_fast_fp64_t abs_f_value;
  mojov_mem_fast_fp64_t updated_guess;
  uint64_t false_value = 0;

  secret_store_fp64(1.0, &guess);
  secret_store_u64(false_value, converged);

  for (unsigned iter = 0; iter < MAXITER; ++iter)
  {
    secret_f(sqrt_value, &guess, &f_value);
    secret_fabs(&f_value, &abs_f_value);
    secret_le_public(&abs_f_value, MAXERR, converged);
    secret_nr_update(sqrt_value, &guess, &updated_guess);
    secret_select_fp64(converged, &guess, &updated_guess, &guess);
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
    mojov_mem_fast_u64_t converged;
    sqrt_value = testdata[i];
    mojov_mem_fast_fp64_t root_ct = nr_solver(&converged);
    double root = mojov_decrypt_fast_fp64(&simon_state, root_ct, CONTRACT_SIG);
    libmin_printf("sqrt(%lf) == %lf (converged:%c)\n",
      sqrt_value,
      root,
      mojov_decrypt_fast_u64(&simon_state, converged, CONTRACT_SIG) ? 't' : 'f');
  }

  libmin_success();
  return 0;
}
