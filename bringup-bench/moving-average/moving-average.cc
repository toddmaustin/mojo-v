#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"

#define DATASET_SIZE 128
#define WINDOW_SIZE 7

typedef struct {
  uint64e_t samples[DATASET_SIZE];
} encrypted_series_t;

static uint64_t plain_samples[DATASET_SIZE];
static encrypted_series_t encrypted_samples;
static uint64e_t encrypted_smoothed[DATASET_SIZE];

static void
build_plain_samples(void)
{
  for (uint64_t i = 0; i < DATASET_SIZE; ++i)
  {
    uint64_t trend = 100 + (i * 3);
    uint64_t seasonal = (i * i + 17 * i + 29) % 41;
    uint64_t burst = ((i % 19) == 0) ? 53 : 0;
    plain_samples[i] = trend + seasonal + burst;
  }
}

static void
encrypt_series(void)
{
  for (uint64_t i = 0; i < DATASET_SIZE; ++i)
    encrypted_samples.samples[i] = plain_samples[i];
}

static void
compute_encrypted_moving_average(void)
{
  uint64e_t window_sum = 0;

  for (uint64_t i = 0; i < WINDOW_SIZE; ++i)
    window_sum = window_sum + encrypted_samples.samples[i];

  for (uint64_t i = 0; i < DATASET_SIZE; ++i)
  {
    uint64_t window_start = (i >= (WINDOW_SIZE - 1)) ? (i - (WINDOW_SIZE - 1)) : 0;
    uint64_t window_end = i;
    uint64_t count = window_end - window_start + 1;

    if (i >= WINDOW_SIZE)
      window_sum = window_sum - encrypted_samples.samples[i - WINDOW_SIZE];

    if (i >= WINDOW_SIZE)
      window_sum = window_sum + encrypted_samples.samples[i];

    if (i < WINDOW_SIZE)
    {
      uint64e_t partial_sum = 0;
      for (uint64_t j = 0; j <= i; ++j)
        partial_sum = partial_sum + encrypted_samples.samples[j];
      encrypted_smoothed[i] = partial_sum / count;
    }
    else
      encrypted_smoothed[i] = window_sum / WINDOW_SIZE;
  }
}

static uint64_t
plain_moving_average_at(uint64_t idx)
{
  uint64_t window_start = (idx >= (WINDOW_SIZE - 1)) ? (idx - (WINDOW_SIZE - 1)) : 0;
  uint64_t sum = 0;
  uint64_t count = idx - window_start + 1;

  for (uint64_t i = window_start; i <= idx; ++i)
    sum += plain_samples[i];

  return sum / count;
}

int
main(void)
{
  if (mojov_configure_kmsm_from_dc_fast() != 0)
    return -1;
  if (mojov_enable_and_verify() != 0)
    return -1;
  if (debug_context(SIMON128_KEY, CONTRACT_SIG) != 0)
    return -1;

  build_plain_samples();
  encrypt_series();

  libmin_printf("moving-average: processing %u encrypted samples with window=%u\n",
    DATASET_SIZE, WINDOW_SIZE);

  compute_encrypted_moving_average();

  libmin_printf("moving-average smoothed dataset:\n");

  for (uint64_t i = 0; i < DATASET_SIZE; ++i)
  {
    uint64_t revealed = encrypted_smoothed[i].decrypt();
    uint64_t expected = plain_moving_average_at(i);

    libmin_printf("  idx=%03lu smooth=%lu\n", i, revealed);

    if (revealed != expected)
    {
      libmin_printf("moving-average ERROR: mismatch at idx=%lu (got=%lu expected=%lu)\n",
        i, revealed, expected);
      return -1;
    }
  }

  libmin_printf("moving-average PASS: validated %u smoothed encrypted samples\n", DATASET_SIZE);

  libmin_success();
  return 0;
}
