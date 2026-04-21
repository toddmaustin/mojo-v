#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"

#define BOARD_SIZE 10

static int solution_count = 0;

// Check if placing a queen at (row, col) is safe.
static uint64e_t
is_safe(uint64e_t queens[], int row, int col)
{
  uint64e_t safe = 1;

  for (int i = 0; i < row; i++)
  {
    uint64e_t q_col = queens[i];
    uint64e_t same_col = (q_col == (uint64_t)col);

    uint64e_t q_col_lt_col = (q_col < (uint64_t)col);
    uint64e_t col_diff_a = (uint64_t)col - q_col;
    uint64e_t col_diff_b = q_col - (uint64_t)col;
    uint64e_t col_diff = cmov(q_col_lt_col, col_diff_a, col_diff_b);

    uint64_t row_diff = (uint64_t)(row - i);
    uint64e_t same_diag = (col_diff == row_diff);

    uint64e_t conflict = same_col || same_diag;
    safe = cmov(conflict, 0ul, safe);
  }

  return safe;
}

// Recursive backtracking solver.
static void
solve(uint64e_t queens[], int row)
{
  if (row == BOARD_SIZE)
  {
    solution_count++;
    return;
  }

  for (int col = 0; col < BOARD_SIZE; col++)
  {
    uint64e_t safe = is_safe(queens, row, col);
    if (safe.decrypt())
    {
      queens[row] = (uint64_t)col;
      solve(queens, row + 1);
    }
  }
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

  uint64e_t *queens = (uint64e_t *)libmin_malloc(BOARD_SIZE * sizeof(uint64e_t));

  solve(queens, 0);

  libmin_printf("Total solutions for %d-Queens: %d\n", BOARD_SIZE, solution_count);

  libmin_free(queens);

  libmin_success();
  return 0;
}
