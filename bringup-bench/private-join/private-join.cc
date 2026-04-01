#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

typedef mojov_mem_fast_u64_t _uint64e_t;
typedef mojov_mem_fast_fp64_t _fp64e_t;
#include "mojov-exo.h"

#define LEFT_ROWS 8
#define RIGHT_ROWS 8

typedef struct {
  uint64e_t key;
  uint64e_t value;
} secret_row_t;

static const uint64_t left_keys[LEFT_ROWS] = {11, 22, 33, 44, 55, 66, 77, 88};
static const uint64_t left_vals[LEFT_ROWS] = {5, 15, 25, 35, 45, 55, 65, 75};

static const uint64_t right_keys[RIGHT_ROWS] = {44, 88, 11, 70, 22, 66, 99, 33};
static const uint64_t right_vals[RIGHT_ROWS] = {100, 200, 300, 400, 500, 600, 700, 800};

static secret_row_t left_secret[LEFT_ROWS];
static secret_row_t right_secret[RIGHT_ROWS];

// Private equi-join mode 1: output joined row count only.
static uint64e_t
join_count_only(void)
{
  uint64e_t count = 0;

  for (unsigned i = 0; i < LEFT_ROWS; ++i)
  {
    for (unsigned j = 0; j < RIGHT_ROWS; ++j)
    {
      uint64e_t matched = (left_secret[i].key == right_secret[j].key);
      count = count + cmov(matched, 1, 0);
    }
  }

  return count;
}

// Private equi-join mode 2: sum values after join.
static uint64e_t
join_sum_after_join(void)
{
  uint64e_t sum = 0;

  for (unsigned i = 0; i < LEFT_ROWS; ++i)
  {
    for (unsigned j = 0; j < RIGHT_ROWS; ++j)
    {
      uint64e_t matched = (left_secret[i].key == right_secret[j].key);
      uint64e_t joined_value = left_secret[i].value + right_secret[j].value;
      sum = sum + cmov(matched, joined_value, 0);
    }
  }

  return sum;
}

// Private equi-join mode 3: sum after join with secret filter.
static uint64e_t
join_with_filter(void)
{
  uint64e_t sum = 0;

  for (unsigned i = 0; i < LEFT_ROWS; ++i)
  {
    for (unsigned j = 0; j < RIGHT_ROWS; ++j)
    {
      uint64e_t matched = (left_secret[i].key == right_secret[j].key);
      uint64e_t joined_value = left_secret[i].value + right_secret[j].value;
      uint64e_t passes_filter = joined_value >= 650;
      uint64e_t selected = matched && passes_filter;
      sum = sum + cmov(selected, joined_value, 0);
    }
  }

  return sum;
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

  for (unsigned i = 0; i < LEFT_ROWS; ++i)
  {
    left_secret[i].key = left_keys[i];
    left_secret[i].value = left_vals[i];
  }

  for (unsigned i = 0; i < RIGHT_ROWS; ++i)
  {
    right_secret[i].key = right_keys[i];
    right_secret[i].value = right_vals[i];
  }

  uint64e_t join_count = join_count_only();
  uint64e_t join_sum = join_sum_after_join();
  uint64e_t filtered_sum = join_with_filter();

  libmin_printf("private-join count-only: %lu\n", join_count.decrypt());
  libmin_printf("private-join sum-after-join: %lu\n", join_sum.decrypt());
  libmin_printf("private-join join-with-filter: %lu\n", filtered_sum.decrypt());

  libmin_success();
  return 0;
}
