#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"
#include "dc-proofcarrying.h"
#include <stdlib.h>

volatile double iszero = 0.0;

#define SECRET

uint16_t mojov_arg;

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

static uint64_t g_contract_sig = CONTRACT_SIG;

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

mojov_mem_proofcarrying_fp64_t
mojov_3rdparty_encrypt(double dblval, uint64_t in_brand, uint64_t contract_sig)
{
  uint64_t auth_sig;
  if (mojov_arg == 25)
  {
    // replay attack, using an old AUTH_SIG
    auth_sig = contract_sig - 1;
  }
  else
    auth_sig = contract_sig;

  uint64_t dfhash = mojov_hash64(mojov_hash64_init(), in_brand);
  mojov_mem_proofcarrying_fp64_t ptval = {.pt = { dblval, ((uint64_t)libmin_rand() << 32) | (uint64_t)libmin_rand(), auth_sig, dfhash} };
  mojov_mem_proofcarrying_fp64_t ctval;

  // encrypt the memory packet with the processor's internal key
  simon_128_128_encrypt(&simon_state, ptval.ct.ct_lo, &ctval.ct.ct_lo);
  simon_128_128_encrypt(&simon_state, (ptval.ct.ct_hi ^ ctval.ct.ct_lo), &ctval.ct.ct_hi);

  return ctval;
}


static double
mojov_3rdparty_decrypt_fp64(mojov_mem_proofcarrying_fp64_t ctval, uint64_t contract_sig)
{
  mojov_mem_proofcarrying_fp64_t ptval;
  simon_128_128_decrypt(&simon_state, ctval.ct.ct_lo, &ptval.ct.ct_lo);
  simon_128_128_decrypt(&simon_state, ctval.ct.ct_hi, &ptval.ct.ct_hi);
  ptval.ct.ct_hi ^= ctval.ct.ct_lo;

  if (ptval.pt.auth_sig != contract_sig)
  {
    libmin_printf("ERROR: mojov_3rdparty_decrypt() decryption validation failed!\n");
    exit(-1);
  }

  return ptval.pt.val;
}

static uint64_t
mojov_3rdparty_decrypt_u64(mojov_mem_proofcarrying_u64_t ctval, uint64_t contract_sig)
{
  mojov_mem_proofcarrying_u64_t ptval;
  simon_128_128_decrypt(&simon_state, ctval.ct.ct_lo, &ptval.ct.ct_lo);
  simon_128_128_decrypt(&simon_state, ctval.ct.ct_hi, &ptval.ct.ct_hi);
  ptval.ct.ct_hi ^= ctval.ct.ct_lo;

  if (ptval.pt.auth_sig != contract_sig)
  {
    libmin_printf("ERROR: mojov_3rdparty_decrypt() decryption validation failed!\n");
    exit(-1);
  }

  return ptval.pt.val;
}

#define mojov_3rdparty_decrypt(ctval, contract_sig) _Generic((ctval),   mojov_mem_proofcarrying_fp64_t: mojov_3rdparty_decrypt_fp64,   mojov_mem_proofcarrying_u64_t: mojov_3rdparty_decrypt_u64 )((ctval), (contract_sig))

// supported sizes: 64, 128, 256 (default), 512, 1024, 2048
#define DATASET_SIZE 64
double raw_data[DATASET_SIZE];
SECRET mojov_mem_proofcarrying_fp64_t secret_data[DATASET_SIZE];

// now sum the array data to coalesce the dataflow hashes
mojov_mem_proofcarrying_fp64_t sum_enc;

// total swaps executed so far
mojov_mem_proofcarrying_u64_t swaps;

void
print_data(double *data, unsigned size)
{
  double sum = 0.0;
  libmin_printf("DATA DUMP:\n");
  for (unsigned i=0; i < size; i++)
  {
    sum += data[i];
    libmin_printf("  data[%4u] = %.20lf, ct =[", i, data[i]);
    secret_print(secret_data[i]);
    libmin_printf("]\n");
  }
  libmin_printf("Total sum = %.20lf\n", sum);
}

void
bubblesort(mojov_mem_proofcarrying_fp64_t *data, unsigned size)
{
  for (unsigned i=0; i < size-1; i++)
  {
    for (unsigned j=0; j < size-1; j++)
    {
      // swap needed?
      if (mojov_arg == 26)
      {
        __asm__ volatile (
          // SECRET bool swap = (data[j] > data[j+1]);
          FLDE(  f28, %0, 0) // data[j]
          FLDE(  f29, %1, 0) // data[j+1]
          "flt.d  x30, f29, f28\n\t" // swap
          // fmv the values for the conditional swap
          "fmv.x.d x28, f28\n\t"
          "fmv.x.d x29, f29\n\t"
          // perform the swap
          // data[j] = secret_cmov(swap, data[j+1], data[j]);
          "czero.nez x31, x29, x30\n\t"
          "czero.eqz x30, x28, x30\n\t"
          "or        x31, x30, x31\n\t"
          "fmv.d.x f30, x31\n\t"
          FSDE(       f30, %0, 0)
          // data[j+1] = secret_cmov(swap, tmp, data[j+1]);
          "flt.d  x30, f29, f28\n\t" // swap
          "czero.nez x31, x28, x30\n\t"
          "czero.eqz x28, x29, x30\n\t"
          "or        x31, x28, x31\n\t" 
          "fmv.d.x f30, x31\n\t"
          FSDE(       f30, %1, 0)
          // count the number of swaps executed
          // swaps = secret_cmov(swap, swaps+1, swaps);
          LDE(  x28, %2, 0) // swaps
          "add x29, x28, x30\n\t" // swaps+1
          // "czero.eqz f31, f28, f30\n\t"
          // "czero.nez f30, f29, f30\n\t"
          // "or        f31, f30, f31\n\t"
          SDE(  x29, %2, 0) // swaps
          :
          : "r" (&data[j]), "r" (&data[j+1]), "r" (&swaps)
          : "x28", "x29", "x30", "x31", "f28", "f29", "f30", "f31" // clobbered registers
        );
      }
      else
      {
        __asm__ volatile (
          // SECRET bool swap = (data[j] > data[j+1]);
          FLDE(  f28, %0, 0) // data[j]
          FLDE(  f29, %1, 0) // data[j+1]
          "flt.d  x30, f29, f28\n\t" // swap
          // fmv the values for the conditional swap
          "fmv.x.d x28, f28\n\t"
          "fmv.x.d x29, f29\n\t"
          // perform the swap
          // data[j] = secret_cmov(swap, data[j+1], data[j]);
          "czero.eqz x31, x29, x30\n\t"
          "czero.nez x30, x28, x30\n\t"
          "or        x31, x30, x31\n\t"
          "fmv.d.x f30, x31\n\t"
          FSDE(       f30, %0, 0)
          // data[j+1] = secret_cmov(swap, tmp, data[j+1]);
          "flt.d  x30, f29, f28\n\t" // swap
          "czero.eqz x31, x28, x30\n\t"
          "czero.nez x28, x29, x30\n\t"
          "or        x31, x28, x31\n\t" 
          "fmv.d.x f30, x31\n\t"
          FSDE(       f30, %1, 0)
          // count the number of swaps executed
          // swaps = secret_cmov(swap, swaps+1, swaps);
          LDE(  x28, %2, 0) // swaps
          "add x29, x28, x30\n\t" // swaps+1
          // "czero.eqz f31, f28, f30\n\t"
          // "czero.nez f30, f29, f30\n\t"
          // "or        f31, f30, f31\n\t"
          SDE(  x29, %2, 0) // swaps
          :
          : "r" (&data[j]), "r" (&data[j+1]), "r" (&swaps)
          : "x28", "x29", "x30", "x31", "f28", "f29", "f30", "f31" // clobbered registers
        );
      }
    }
  }

  // clean sum_enc
  __asm__ volatile (
    "fmv.d.x  f28, x0\n\t"
    FSDE     (f28, %0, 0)
    :
    : "r" (&sum_enc)
    : "f28" // clobbered registers
  );

  for (unsigned i=0; i < size; i++)
  {
    if (mojov_arg == 20) 
    {
      __asm__ volatile (
        FLDE     (f29, %0, 0)         // load data[i]

        // DFHASH TEST: change fadd.d to fsub.d
        "fsub.d   f28, f28, f29\n\t"  // add to sum_enc
        :
        : "r" (&data[i])
        : "f28", "f29" // clobbered registers
      );
    }
    else if (mojov_arg == 21)
    {
      __asm__ volatile (
        FLDE     (f29, %0, 0)         // load data[i]

        // DFHASH TEST: change any operand order
        "fadd.d   f28, f29, f28\n\t"  // add to sum_enc
        :
        : "r" (&data[i])
        : "f28", "f29" // clobbered registers
      );
    }
    else
    {
      __asm__ volatile (
        FLDE     (f29, %0, 0)         // load data[i]
        "fadd.d   f28, f28, f29\n\t"  // add to sum_enc
        :
        : "r" (&data[i])
        : "f28", "f29" // clobbered registers
      );
    }
  }

  // save sum_enc to memory
  __asm__ volatile (
    FSDE  (f28, %0, 0)
    :
    : "r" (&sum_enc)
    : "f28" // clobbered registers
  );

}

int
main(void)
{
  if (mojov_configure_kmsm_from_dc_proof() != 0)
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

  // read the mojov_arg
  mojov_arg = (val >> 12) & 0xffff;

  // enable private register semantics (bit 0 = 1)
  if (mojov_enable_and_verify() != 0)
    return -1;

  if (g_contract_sig == 0)
  {
    libmin_printf("ERROR: missing compile-time CONTRACT_SIG.\n");
    return -1;
  }

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

  // initialize the array to sort with 3rd party input-branded data
  for (unsigned i=0; i < DATASET_SIZE; i++)
  {
    if (mojov_arg == 1)
    {
      // DFHASH TEST: modifying input data should not affect dfhash
      raw_data[i] = genrand_fp64() * 0.67;
    }
    else if (mojov_arg == 2)
    {
      // DFHASH TEST: modifying input data should not affect dfhash
      raw_data[i] = genrand_fp64() + genrand_fp64(); 
    }
    else if (mojov_arg == 3)
    {
      // DFHASH TEST: modifying input data should not affect dfhash
      raw_data[i] = 0.0; 
    }
    else
      raw_data[i] = genrand_fp64();

    if (mojov_arg == 23 && i == 14)
    {
      // substitution attack
      secret_data[i] = mojov_3rdparty_encrypt(raw_data[i], /* input type */10000000, g_contract_sig);
    }
    else
      secret_data[i] = mojov_3rdparty_encrypt(raw_data[i], /* input type */i, g_contract_sig);
  }
  print_data(raw_data, DATASET_SIZE);

  if (mojov_arg == 24)
  {
    // swap array elements 12 and 13
    mojov_mem_proofcarrying_fp64_t tmp = secret_data[12];
    secret_data[12] = secret_data[13];
    secret_data[13] = tmp;
  }

  if (mojov_arg == 27)
  {
    // copy attack
    secret_data[12] = secret_data[13];
  }

  // DFHASH baseline test
  {
    // performance monitoring
    // uint64_t icnt_start = __instret();

    bubblesort(secret_data, DATASET_SIZE);

    // uint64_t icnt_end = __instret();
    // libmin_printf("INFO: bubblesort inst count = %lu.\n", icnt_end - icnt_start + 1);
  }


  // decrypt the array
  for (unsigned i=0; i < DATASET_SIZE; i++)
    raw_data[i] = mojov_3rdparty_decrypt(secret_data[i], g_contract_sig);
  print_data(raw_data, DATASET_SIZE);

  libmin_printf("INFO: %lu swaps executed.\n", mojov_3rdparty_decrypt(swaps, g_contract_sig));

  // check the array
  bool sorted = true;
  for (unsigned i=0; i < DATASET_SIZE-1; i++)
  {
    if (raw_data[i] > raw_data[i+1])
    {
      sorted = false;
      break;
    }
  }
  libmin_printf("ERROR: data is %sproperly sorted.\n", sorted ? "" : "not ");

  double sum = mojov_3rdparty_decrypt(sum_enc, g_contract_sig);
  libmin_printf("INFO: final summary variable: %.20lf\n", sum);

  uint64_t dfhash;
  if (mojov_arg == 22)
  {
    dfhash = mojov_dfhash_proofcarrying_fp64(&simon_state, secret_data[DATASET_SIZE-1]);
    libmin_printf("INFO: final dataflow hash: 0x%08x%08x\n", (uint32_t)(dfhash >> 32), (uint32_t)dfhash);
  }
  else
  {
    dfhash = mojov_dfhash_proofcarrying_fp64(&simon_state, sum_enc);
    libmin_printf("INFO: final dataflow hash: 0x%08x%08x\n", (uint32_t)(dfhash >> 32), (uint32_t)dfhash);
  }
  if (dfhash == 0xc928d654cf18433e)
    libmin_printf("INFO: secret computation integrity is INTACT.\n");
  else
    libmin_printf("INFO: secret computation integrity is CORRUPTED! (expected: 0xc928d654cf18433e)\n");


  libmin_success();
  return 0;
}
