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
  fp64e_t samples[DATASET_SIZE];
} encrypted_series_t;

static double plain_samples[DATASET_SIZE];
static encrypted_series_t encrypted_samples;
static fp64e_t encrypted_smoothed[DATASET_SIZE];

static double
abs_diff(double a, double b)
{
  return (a > b) ? (a - b) : (b - a);
}

static void
build_plain_samples(void)
{
  for (uint64_t i = 0; i < DATASET_SIZE; ++i)
  {
    double trend = 100.0 + ((double)i * 1.75);
    double seasonal = (double)((i * i + 17 * i + 29) % 41) * 0.125;
    double burst = ((i % 19) == 0) ? 2.5 : 0.0;
    double jitter = (double)((i * 7) % 13) * 0.01;
    plain_samples[i] = trend + seasonal + burst + jitter;
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
  fp64e_t window_sum = 0.0;

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
      fp64e_t partial_sum = 0.0;
      for (uint64_t j = 0; j <= i; ++j)
        partial_sum = partial_sum + encrypted_samples.samples[j];
      encrypted_smoothed[i] = partial_sum / (double)count;
    }
    else
      encrypted_smoothed[i] = window_sum / (double)WINDOW_SIZE;
  }
}

static double
plain_moving_average_at(uint64_t idx)
{
  uint64_t window_start = (idx >= (WINDOW_SIZE - 1)) ? (idx - (WINDOW_SIZE - 1)) : 0;
  double sum = 0.0;
  uint64_t count = idx - window_start + 1;

  for (uint64_t i = window_start; i <= idx; ++i)
    sum += plain_samples[i];

  return sum / (double)count;
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

  libmin_printf("moving-average-fp64: processing %u encrypted FP64 samples with window=%u\n",
    DATASET_SIZE, WINDOW_SIZE);

  compute_encrypted_moving_average();

  libmin_printf("moving-average-fp64 smoothed dataset:\n");

  for (uint64_t i = 0; i < DATASET_SIZE; ++i)
  {
    double revealed = encrypted_smoothed[i].decrypt();
    double expected = plain_moving_average_at(i);

    libmin_printf("  idx=%03lu smooth=%.6f\n", i, revealed);

    if (abs_diff(revealed, expected) > 1.0e-9)
    {
      libmin_printf("moving-average-fp64 ERROR: mismatch at idx=%lu (got=%.12f expected=%.12f)\n",
        i, revealed, expected);
      return -1;
    }
  }

  libmin_printf("moving-average-fp64 PASS: validated %u smoothed encrypted FP64 samples\n", DATASET_SIZE);

  libmin_success();
  return 0;
}
