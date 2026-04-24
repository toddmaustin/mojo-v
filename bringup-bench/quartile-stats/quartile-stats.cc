#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"

#define DATA_SIZE 128
#define QUARTILES 4
#define QUARTILE_SIZE (DATA_SIZE / QUARTILES)

static const uint64_t plain_data[DATA_SIZE] = {
  912, 43, 751, 619, 88, 305, 1440, 271, 532, 998, 64, 417, 1205, 349, 781, 56,
  663, 214, 1402, 905, 117, 488, 73, 1106, 529, 358, 892, 177, 1364, 441, 629, 82,
  1003, 255, 713, 160, 845, 392, 1277, 509, 298, 942, 121, 684, 333, 1510, 570, 206,
  799, 95, 1168, 452, 617, 274, 1331, 38, 720, 540, 109, 970, 410, 1462, 681, 248,
  885, 327, 1194, 74, 560, 431, 1048, 219, 776, 597, 142, 1259, 468, 291, 918, 61,
  703, 354, 1388, 507, 164, 832, 446, 1021, 236, 674, 389, 1497, 522, 113, 760, 315,
  940, 201, 1296, 578, 87, 690, 363, 1079, 252, 814, 425, 1435, 548, 176, 969, 337,
  1217, 93, 655, 280, 1112, 472, 736, 59, 138, 861, 504, 1150, 347, 1502, 621, 263
};

static uint64e_t secret_data[DATA_SIZE];

static void
secret_compare_swap(uint64e_t *a, uint64e_t *b)
{
  uint64e_t swap = (*a > *b);
  uint64e_t lo = cmov(swap, *b, *a);
  uint64e_t hi = cmov(swap, *a, *b);
  *a = lo;
  *b = hi;
}

static void
secret_sort(uint64e_t data[DATA_SIZE])
{
  for (uint64_t i = 0; i < DATA_SIZE; ++i)
    for (uint64_t j = 0; j + 1 < DATA_SIZE - i; ++j)
      secret_compare_swap(&data[j], &data[j + 1]);
}

static void
compute_secret_quartile_stats(const uint64e_t sorted[DATA_SIZE], uint64e_t cuts[3], uint64e_t avgs[QUARTILES])
{
  cuts[0] = sorted[(DATA_SIZE / 4) - 1];
  cuts[1] = sorted[(DATA_SIZE / 2) - 1];
  cuts[2] = sorted[((3 * DATA_SIZE) / 4) - 1];

  uint64e_t sums[QUARTILES] = {(uint64e_t)0, (uint64e_t)0, (uint64e_t)0, (uint64e_t)0};

  for (uint64_t i = 0; i < DATA_SIZE; ++i)
  {
    uint64_t q = i / QUARTILE_SIZE;
    sums[q] = sums[q] + sorted[i];
  }

  for (uint64_t q = 0; q < QUARTILES; ++q)
    avgs[q] = sums[q] / (uint64e_t)QUARTILE_SIZE;
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

  for (uint64_t i = 0; i < DATA_SIZE; ++i)
    secret_data[i] = plain_data[i];

  secret_sort(secret_data);

  uint64e_t quartile_cuts[3];
  uint64e_t quartile_avgs[QUARTILES];
  compute_secret_quartile_stats(secret_data, quartile_cuts, quartile_avgs);

  uint64e_t secret_min = secret_data[0];
  uint64e_t secret_max = secret_data[DATA_SIZE - 1];

  libmin_printf("quartile-stats: encrypted quartile analysis complete\n");
  libmin_printf("input summary: samples=%u range=[%lu, %lu]\n",
    DATA_SIZE, secret_min.decrypt(), secret_max.decrypt());
  libmin_printf("quartile cuts order positions (1-based): Q1=%u Q2=%u Q3=%u\n",
    (unsigned)(DATA_SIZE / 4), (unsigned)(DATA_SIZE / 2), (unsigned)((3 * DATA_SIZE) / 4));
  libmin_printf("quartile cuts order positions (0-based): Q1=%u Q2=%u Q3=%u\n",
    (unsigned)((DATA_SIZE / 4) - 1), (unsigned)((DATA_SIZE / 2) - 1), (unsigned)(((3 * DATA_SIZE) / 4) - 1));
  libmin_printf("quartile cuts (Q1,Q2,Q3): %lu, %lu, %lu\n",
    quartile_cuts[0].decrypt(), quartile_cuts[1].decrypt(), quartile_cuts[2].decrypt());
  libmin_printf("quartile averages (Q1,Q2,Q3,Q4): %lu, %lu, %lu, %lu\n",
    quartile_avgs[0].decrypt(), quartile_avgs[1].decrypt(), quartile_avgs[2].decrypt(), quartile_avgs[3].decrypt());

  libmin_success();
  return 0;
}
