#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"
#include "mojov-math.h"

#define NUM_NODES 8
#define MAX_ITERS 40
#define DAMPING 0.85
#define THRESHOLD 1.0e-6

static const uint8_t plain_adj[NUM_NODES][NUM_NODES] = {
  {0, 1, 1, 0, 0, 0, 0, 0},
  {0, 0, 1, 1, 0, 0, 0, 0},
  {1, 0, 0, 1, 1, 0, 0, 0},
  {0, 0, 0, 0, 1, 1, 0, 0},
  {0, 0, 0, 0, 0, 1, 1, 0},
  {0, 0, 0, 0, 0, 0, 1, 1},
  {1, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 0, 0, 0, 0, 0, 0}
};

static int8e_t adj[NUM_NODES][NUM_NODES];
static uint64e_t out_degree[NUM_NODES];
static fp64e_t ranks[NUM_NODES];
static fp64e_t next_ranks[NUM_NODES];

static void
initialize_encrypted_state(void)
{
  fp64e_t inv_n = (fp64e_t)1.0 / (fp64e_t)NUM_NODES;

  for (unsigned i = 0; i < NUM_NODES; ++i)
  {
    uint64e_t deg = 0;
    for (unsigned j = 0; j < NUM_NODES; ++j)
    {
      adj[i][j] = (int8e_t)plain_adj[i][j];
      deg = deg + (uint64e_t)plain_adj[i][j];
    }

    out_degree[i] = deg;
    ranks[i] = inv_n;
    next_ranks[i] = (fp64e_t)0.0;
  }
}

static void
pagerank_pull_iteration(void)
{
  fp64e_t base = ((fp64e_t)1.0 - (fp64e_t)DAMPING) / (fp64e_t)NUM_NODES;

  for (unsigned i = 0; i < NUM_NODES; ++i)
    next_ranks[i] = base;

  for (unsigned src = 0; src < NUM_NODES; ++src)
  {
    if (out_degree[src].decrypt() == 0)
    {
      fp64e_t spread = ((fp64e_t)DAMPING * ranks[src]) / (fp64e_t)NUM_NODES;
      for (unsigned dst = 0; dst < NUM_NODES; ++dst)
        next_ranks[dst] = next_ranks[dst] + spread;
      continue;
    }

    fp64e_t push = ((fp64e_t)DAMPING * ranks[src]) / (fp64e_t)out_degree[src];
    for (unsigned dst = 0; dst < NUM_NODES; ++dst)
    {
      uint64e_t has_edge = ((uint64e_t)adj[src][dst]) == (uint64e_t)1;
      next_ranks[dst] = next_ranks[dst] + cmov(has_edge, push, (fp64e_t)0.0);
    }
  }
}

static uint64e_t
commit_and_check_convergence(void)
{
  uint64e_t changed = 0;

  for (unsigned i = 0; i < NUM_NODES; ++i)
  {
    fp64e_t diff = mojov_fabs(next_ranks[i] - ranks[i]);
    changed = changed || (diff > (fp64e_t)THRESHOLD);
    ranks[i] = next_ranks[i];
  }

  return changed;
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

  initialize_encrypted_state();

  unsigned iter = 0;
  while (iter < MAX_ITERS)
  {
    pagerank_pull_iteration();
    uint64e_t changed = commit_and_check_convergence();
    ++iter;
    if (!changed.decrypt())
      break;
  }

  fp64e_t sum = 0.0;
  for (unsigned i = 0; i < NUM_NODES; ++i)
    sum = sum + ranks[i];

  libmin_printf("pagerank iterations: %u\n", iter);
  libmin_printf("pagerank sum: %.9f\n", sum.decrypt());
  for (unsigned i = 0; i < NUM_NODES; ++i)
  {
    fp64e_t norm = ranks[i] / sum;
    libmin_printf("node %u rank %.9f\n", i, norm.decrypt());
  }

  libmin_success();
  return 0;
}
