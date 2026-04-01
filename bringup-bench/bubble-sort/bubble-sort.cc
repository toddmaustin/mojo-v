#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
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
bubblesort(uint64e_t *data, unsigned size)
{
  for (unsigned i=0; i < size-1; i++)
  {
    for (unsigned j=0; j < size - i - 1; j++)
    {
      uint64e_t do_swap = data[j+1] < data[j];
      uint64e_t tmp = data[j];
      data[j] = cmov(do_swap, data[j+1], data[j]);
      data[j+1] = cmov(do_swap, tmp, data[j+1]);
      swaps = cmov(do_swap, swaps+1, swaps);
    }
  }
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

  // initialize swaps
  swaps = 0;

  // initialize the array to sort
  for (unsigned i=0; i < DATASET_SIZE; i++)
    secret_data[i] = raw_data[i] = libmin_rand();
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
    raw_data[i] = secret_data[i].decrypt();
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
  libmin_printf("INFO: %lu swaps executed.\n", swaps.decrypt());
  libmin_printf("INFO: data is properly sorted.\n");

  libmin_success();
  return 0;
}
