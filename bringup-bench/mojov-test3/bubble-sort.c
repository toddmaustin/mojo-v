#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"
#include "dc-fast.h"

#define SECRET

// SECRET int
// secret_cmov(SECRET bool p, SECRET int x, SECRET int y)
// {
//   return (int)p*x + (int)!p*y;
// }


uint128_t simon_key = SIMON128_KEY;
simon_state_t simon_state;


static
int rb(int n)                 // uniform in [0..n-1], no modulo bias
{
  int limit = RAND_MAX - (RAND_MAX % n);
  int r; do { r = libmin_rand(); } while (r > limit);
  return r % n;
}

double
genrand_fp64(void)
{
  int idigits = 0 + rb(11);         // 0..10
  double v = 0.0;

  // integer part: first digit 1..9, then (idigits-1) digits 0..9
  v = (double)(1 + rb(9));
  for (int i = 1; i < idigits; ++i)
    v = v * 10.0 + (double)rb(10);

  // fractional part: exactly 5 digits
  double frac = 0.0;
  for (int i = 0; i < 5; ++i)
    frac = frac * 10.0 + (double)rb(10);

  v += frac / 100000.0;              // add 5-digit fraction

  // random sign
  if (rb(2)) v = -v;
  return v;
}

// supported sizes: 256 (default), 512, 1024, 2048
#define DATASET_SIZE 256
double raw_data[DATASET_SIZE];
SECRET mojov_mem_fast_fp64_t secret_data[DATASET_SIZE];

// total swaps executed so far
mojov_mem_fast_u64_t swaps;

void
print_data(double *data, unsigned size)
{
  libmin_printf("DATA DUMP:\n");
  for (unsigned i=0; i < size; i++)
  {
    libmin_printf("  data[%4u] = %.20lf\n", i, data[i]);
  }
}

void
bubblesort(mojov_mem_fast_fp64_t *data, unsigned size)
{
  for (unsigned i=0; i < size-1; i++)
  {
    for (unsigned j=0; j < size-1; j++)
    {
      // swap needed?
      __asm__ volatile (
        // SECRET bool swap = (data[j] > data[j+1]);
        FLDE(  f28, %0, 0) // data[j]
        FLDE(  f29, %1, 0) // data[j+1]
        "flt.d  t5, f29, f28\n\t" // swap
        // fmv the values for the conditional swap
        "fmv.x.d t3, f28\n\t"
        "fmv.x.d t4, f29\n\t"
        // perform the swap
        // data[j] = secret_cmov(swap, data[j+1], data[j]);
        "czero.eqz t6, t4, t5\n\t"
        "czero.nez t5, t3, t5\n\t"
        "or        t6, t5, t6\n\t"
        "fmv.d.x f30, t6\n\t"
        FSDE(       f30, %0, 0)
        // data[j+1] = secret_cmov(swap, tmp, data[j+1]);
        "flt.d  t5, f29, f28\n\t" // swap
        "czero.eqz t6, t3, t5\n\t"
        "czero.nez t3, t4, t5\n\t"
        "or        t6, t3, t6\n\t" 
        "fmv.d.x f30, t6\n\t"
        FSDE(       f30, %1, 0)
        // count the number of swaps executed
        // swaps = secret_cmov(swap, swaps+1, swaps);
        LDE(  t3, %2, 0) // swaps
        "add t4, t3, t5\n\t" // swaps+1
        // "czero.eqz f31, f28, f30\n\t"
        // "czero.nez f30, f29, f30\n\t"
        // "or        f31, f30, f31\n\t"
        SDE(  t4, %2, 0) // swaps
        :
        : "r" (&data[j]), "r" (&data[j+1]), "r" (&swaps)
        : "t3", "t4", "t5", "t6", "f28", "f29", "f30", "f31" // clobbered registers
      );
    }
  }
}

int
main(void)
{
  if (mojov_configure_kmsm_from_dc_fast() != 0)
    return -1;

  // initilize cipher engine, for checking results
  simon_128_128_keyexpand(&simon_state, simon_key, 68);

  //
  // mprivregcfg tests
  //
  libmin_printf("** Running CSR[privreg] tests...\n");

  uint64_t val;

  // read reset value
  val = mojov_read_mprivregcfg();
  libmin_printf("Initial mprivregcfg = 0x%lx, ", val);
  mojov_print_mprivregcfg(val);
  libmin_printf("\n");

  // enable private register semantics (bit 0 = 1)
  if (mojov_enable_and_verify() != 0)
    return -1;

  val = mojov_read_mprivregcfg();
  libmin_printf("After enable, mprivregcfg = 0x%lx, ", val);
  mojov_print_mprivregcfg(val);
  libmin_printf("\n");

  // initialize the pseudo-RNG
  libmin_srand(42);

  // initialize swaps
  // swaps = 0;
  __asm__ volatile (
    "mv   t3, x0\n\t"
    SDE  (t3, %0, 0)
    :
    : "r" (&swaps)
    : "t3" // clobbered registers
  );

  // initialize the array to sort
  for (unsigned i=0; i < DATASET_SIZE; i++)
  {
    raw_data[i] = genrand_fp64();
    __asm__ volatile (
      // secret_data[i] = raw_data[i];
      "fld   f28, (%0)\n\t"
      FSDE(  f28, %1, 0)
      :
      : "r" (&raw_data[i]), "r" (&secret_data[i])
      : "f28" // clobbered registers
    );
  }
  print_data(raw_data, DATASET_SIZE);

  {
    // performance monitoring
    // uint64_t icnt_start = __instret();

    bubblesort(secret_data, DATASET_SIZE);

    // uint64_t icnt_end = __instret();
    // libmin_printf("INFO: bubblesort inst count = %lu.\n", icnt_end - icnt_start + 1);
  }


  // decrypt the array
  for (unsigned i=0; i < DATASET_SIZE; i++)
    raw_data[i] = mojov_decrypt_fast_fp64(&simon_state, secret_data[i], CONTRACT_SIG);
  print_data(raw_data, DATASET_SIZE);

  // check the array
  for (unsigned i=0; i < DATASET_SIZE-1; i++)
  {
    if (raw_data[i] > raw_data[i+1])
    {
      libmin_printf("ERROR: data is not properly sorted.\n");
      return -1;
    }
  }
  libmin_printf("INFO: %lu swaps executed.\n", mojov_decrypt_fast_u64(&simon_state, swaps, CONTRACT_SIG));
  libmin_printf("INFO: data is properly sorted.\n");

  libmin_success();
  return 0;
}
