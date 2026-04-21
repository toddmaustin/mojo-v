#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"

namespace {

constexpr unsigned kRows = 20;
constexpr unsigned kCols = 20;
constexpr unsigned kIterations = 250;

uint64e_t board_a[kRows][kCols];
uint64e_t board_b[kRows][kCols];

using board_t = uint64e_t[kRows][kCols];

inline unsigned wrap_row(int r) {
  return static_cast<unsigned>((r + static_cast<int>(kRows)) % static_cast<int>(kRows));
}

inline unsigned wrap_col(int c) {
  return static_cast<unsigned>((c + static_cast<int>(kCols)) % static_cast<int>(kCols));
}

void print_board(unsigned i, board_t& board) {
  libmin_printf("HighLife %ux%u after %u iterations:\n", kRows, kCols, i);

  unsigned alive_count = 0;
  for (unsigned r = 0; r < kRows; ++r) {
    for (unsigned c = 0; c < kCols; ++c) {
      const uint64_t cell = board[r][c].decrypt();
      const char ch = cell ? '#' : '.';
      libmin_printf("%c", ch);
      alive_count += static_cast<unsigned>(cell);
    }
    libmin_printf("\n");
  }

  libmin_printf("Alive cells: %u\n", alive_count);
}

void init_board() {
  for (unsigned r = 0; r < kRows; ++r) {
    for (unsigned c = 0; c < kCols; ++c) {
      const uint64_t v = libmin_rand() & 1u;
      board_a[r][c] = v;
      board_b[r][c] = 0;
    }
  }
}

uint64e_t neighbor_count(board_t& board, unsigned r, unsigned c) {
  uint64e_t n = 0;
  for (int dr = -1; dr <= 1; ++dr) {
    for (int dc = -1; dc <= 1; ++dc) {
      if (dr == 0 && dc == 0) {
        continue;
      }
      n += board[wrap_row(static_cast<int>(r) + dr)][wrap_col(static_cast<int>(c) + dc)];
    }
  }
  return n;
}

void step_highlife(board_t& cur, board_t& nxt) {
  for (unsigned r = 0; r < kRows; ++r) {
    for (unsigned c = 0; c < kCols; ++c) {
      const uint64e_t alive = cur[r][c];
      const uint64e_t n = neighbor_count(cur, r, c);
      const uint64e_t survive = (n == 2) | (n == 3);
      const uint64e_t born = (n == 3) | /* extra highlife birth condition */(n == 6);
      nxt[r][c] = cmov(alive, survive, born);
    }
  }
}

board_t& run_simulation() {
  board_t* cur = &board_a;
  board_t* nxt = &board_b;

  for (unsigned i = 0; i < kIterations; ++i) {
    if ((i % 25) == 0)
      print_board(i, *cur);
    step_highlife(*cur, *nxt);
    board_t* tmp = cur;
    cur = nxt;
    nxt = tmp;
  }

  return *cur;
}

}  // namespace

int main(void) {
  if (mojov_configure_kmsm_from_dc_fast() != 0)
    return -1;

  if (mojov_enable_and_verify() != 0)
    return -1;

  if (debug_context(SIMON128_KEY, CONTRACT_SIG) != 0)
    return -1;

  // initialize RNG
  libmin_srand(42);

  init_board();
  board_t& final_board = run_simulation();
  print_board(kIterations, final_board);

  libmin_success();
  return 0;
}
