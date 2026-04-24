#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"

#define TSP_N 8
#define TSP_MASKS (1u << TSP_N)
#define TSP_INF  ((uint64_t)1000000000ULL)

static uint64e_t graph[TSP_N][TSP_N];
static uint64_t _graph[TSP_N][TSP_N] = {
  { 0, 29, 20, 21, 16, 31, 100, 12 },
  { 29, 0, 15, 29, 28, 40, 72, 21 },
  { 20, 15, 0, 15, 14, 25, 81, 9 },
  { 21, 29, 15, 0, 4, 12, 92, 12 },
  { 16, 28, 14, 4, 0, 16, 94, 9 },
  { 31, 40, 25, 12, 16, 0, 95, 24 },
  { 100, 72, 81, 92, 94, 95, 0, 90 },
  { 12, 21, 9, 12, 9, 24, 90, 0 }
};

// dp[mask][j] = minimum cost to start at 0, visit "mask", and end at j
static uint64e_t dp[TSP_MASKS][TSP_N];
// parent[mask][j] = previous city before j on best path for state (mask, j)
static uint64e_t parent[TSP_MASKS][TSP_N];

static inline void
heldkarp_tsp(void)
{
  for (unsigned mask = 0; mask < TSP_MASKS; ++mask)
    for (unsigned city = 0; city < TSP_N; ++city) {
      dp[mask][city] = TSP_INF;
      parent[mask][city] = 0;
    }

  dp[1u << 0][0] = 0;

  for (unsigned mask = 0; mask < TSP_MASKS; ++mask) {
    if ((mask & 1u) == 0)
      continue;

    for (unsigned end = 0; end < TSP_N; ++end) {
      if ((mask & (1u << end)) == 0)
        continue;
      if (end == 0 && mask != (1u << 0))
        continue;

      unsigned prev_mask = mask ^ (1u << end);
      if (prev_mask == 0 && end != 0)
        continue;

      uint64e_t best = dp[mask][end];
      uint64e_t best_parent = parent[mask][end];

      if (end == 0 && mask == (1u << 0)) {
        best = 0;
        best_parent = 0;
      } else {
        for (unsigned prev = 0; prev < TSP_N; ++prev) {
          if ((prev_mask & (1u << prev)) == 0)
            continue;

          uint64e_t cand = dp[prev_mask][prev] + graph[prev][end];
          uint64e_t better = (cand < best);
          best = cmov(better, cand, best);
          best_parent = cmov(better, (uint64e_t)prev, best_parent);
        }
      }

      dp[mask][end] = best;
      parent[mask][end] = best_parent;
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

  libmin_srand(42);

  for (unsigned i = 0; i < TSP_N; ++i)
    for (unsigned j = 0; j < TSP_N; ++j)
      graph[i][j] = _graph[i][j];

  heldkarp_tsp();

  const unsigned full_mask = TSP_MASKS - 1u;
  uint64e_t best_cycle = TSP_INF;
  uint64e_t best_last = 0;

  for (unsigned end = 1; end < TSP_N; ++end) {
    uint64e_t tour = dp[full_mask][end] + graph[end][0];
    uint64e_t better = (tour < best_cycle);
    best_cycle = cmov(better, tour, best_cycle);
    best_last = cmov(better, (uint64e_t)end, best_last);
  }

  // Result decryption only after all encrypted processing is complete.
  libmin_printf("Held-Karp TSP benchmark (encrypted graph/state)\n");
  libmin_printf("Cities: %d\n", TSP_N);
  libmin_printf("Minimum Hamiltonian cycle cost: %llu\n", best_cycle.decrypt());

  // Reconstruct and print one optimal tour (decrypts occur only while printing).
  libmin_printf("Tour: 0");
  unsigned mask = full_mask;
  unsigned cur = (unsigned)best_last.decrypt();

  for (unsigned step = 1; step < TSP_N; ++step) {
    libmin_printf(" -> %u", cur);
    unsigned prev = (unsigned)parent[mask][cur].decrypt();
    mask ^= (1u << cur);
    cur = prev;
  }
  libmin_printf(" -> 0\n");

  libmin_success();
  return 0;
}
