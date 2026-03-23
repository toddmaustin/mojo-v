#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"
uint128_t simon_key = SIMON128_KEY;
simon_state_t simon_state;

typedef mojov_mem_fast_u64_t _uint64e_t;
typedef mojov_mem_fast_fp64_t _fp64e_t;
#include "mojov-intrinsics.h"

// supported sizes: 256 (default), 512, 1024, 2048
#define DATASET_SIZE 256
uint64_t raw_data[DATASET_SIZE];
_uint64e_t secret_data[DATASET_SIZE];

// total swaps executed so far
_uint64e_t swaps;

void
print_data(uint64_t *data, unsigned size)
{
  libmin_printf("DATA DUMP:\n");
  for (unsigned i=0; i < size; i++)
  {
    libmin_printf("  data[%4u] = %10ld, ct =[", i, data[i]);
    secret_print(secret_data[i]);
    libmin_printf("]\n");
  }
}

void
bubblesort(_uint64e_t *data, unsigned size)
{
  for (unsigned i=0; i < size-1; i++)
  {
    for (unsigned j=0; j < size - i - 1; j++)
    {
      _uint64e_t do_swap = _sltu(data[j+1], data[j]);
      _uint64e_t tmp = data[j];
      data[j] = _cmov(do_swap, data[j+1], data[j]);
      data[j+1] = _cmov(do_swap, tmp, data[j+1]);
      swaps = _cmov(do_swap, _addi(swaps, 1), swaps);
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
  swaps = _enc(0);

  // initialize the array to sort
  for (unsigned i=0; i < DATASET_SIZE; i++)
  {
    raw_data[i] = libmin_rand();
    secret_data[i] = _enc(raw_data[i]);
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
