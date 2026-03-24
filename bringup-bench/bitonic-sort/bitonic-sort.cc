#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"
uint128_t simon_key = SIMON128_KEY;
simon_state_t simon_state;

typedef mojov_mem_fast_u64_t _uint64e_t;
typedef mojov_mem_fast_fp64_t _fp64e_t;
#include "mojov-exo.h"

// supported sizes: 256 (default), 512, 1024, 2048
#define DATASET_SIZE 256
uint64_t raw_data[DATASET_SIZE];
uint64e_t secret_data[DATASET_SIZE];

// total swaps executed so far
uint64e_t swaps;

void
print_data(uint64_t *data, unsigned size)
{
  libmin_printf("DATA DUMP:\n");
  for (unsigned i=0; i < size; i++)
  {
    libmin_printf("  data[%4u] = %10ld\n", i, data[i]);
  }
}

void
bitonic_sort(uint64e_t *data, unsigned size)
{
  for (unsigned k = 2; k <= size; k <<= 1) // k is doubled every iteration
  {
    for (unsigned j = k/2; j > 0; j >>= 1) // j is halved at every iteration, with truncation of fractional parts
    {
      for (unsigned i = 0; i < size; i++)
      {
        unsigned l = (i ^ j);
        uint64e_t pred = ((l > i) &&
                          (((uint64e_t)((i & k) == 0) && (data[l] < data[i])) || ((uint64e_t)((i & k) != 0) && (data[i] < data[l])))
                         );
        uint64e_t tmp = data[i];
        data[i] = cmov(pred, data[l], data[i]);
        data[l] = cmov(pred, tmp, data[l]);
        swaps = swaps + 1;
      }
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
  swaps = 0;

  // initialize the array to sort
  for (unsigned i=0; i < DATASET_SIZE; i++)
  {
    secret_data[i] = raw_data[i] = libmin_rand();
  }
  print_data(raw_data, DATASET_SIZE);

  {
    // performance monitoring
    // uint64_t icnt_start = __instret();

    bitonic_sort(secret_data, DATASET_SIZE);

    // uint64_t icnt_end = __instret();
    // libmin_printf("INFO: bubblesort inst count = %lu.\n", icnt_end - icnt_start + 1);
  }


  // decrypt the array
  for (unsigned i=0; i < DATASET_SIZE; i++)
    raw_data[i] = mojov_decrypt_fast_u64(&simon_state, secret_data[i], CONTRACT_SIG);
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
