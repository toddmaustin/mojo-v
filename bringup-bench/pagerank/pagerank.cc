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

// VIP type mapping for Mojo-V bringup.
typedef int8e_t VIP_ENCCHAR;
typedef uint64e_t VIP_ENCBOOL;
typedef uint64e_t VIP_ENCUINT64;
typedef fp32e_t VIP_ENCFLOAT;
typedef fp64e_t VIP_ENCDOUBLE;

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

static VIP_ENCCHAR adj[NUM_NODES][NUM_NODES];
static VIP_ENCUINT64 out_degree[NUM_NODES];
static VIP_ENCDOUBLE ranks[NUM_NODES];
static VIP_ENCDOUBLE next_ranks[NUM_NODES];

static void
initialize_encrypted_state(void)
{
  VIP_ENCDOUBLE inv_n = (VIP_ENCDOUBLE)1.0 / (VIP_ENCDOUBLE)NUM_NODES;

  for (unsigned i = 0; i < NUM_NODES; ++i)
  {
    VIP_ENCUINT64 deg = 0;
    for (unsigned j = 0; j < NUM_NODES; ++j)
    {
      adj[i][j] = (VIP_ENCCHAR)plain_adj[i][j];
      deg = deg + (VIP_ENCUINT64)plain_adj[i][j];
    }

    out_degree[i] = deg;
    ranks[i] = inv_n;
    next_ranks[i] = (VIP_ENCDOUBLE)0.0;
  }
}

static void
pagerank_pull_iteration(void)
{
  VIP_ENCDOUBLE base = ((VIP_ENCDOUBLE)1.0 - (VIP_ENCDOUBLE)DAMPING) / (VIP_ENCDOUBLE)NUM_NODES;

  for (unsigned i = 0; i < NUM_NODES; ++i)
    next_ranks[i] = base;

  for (unsigned src = 0; src < NUM_NODES; ++src)
  {
    if (out_degree[src].decrypt() == 0)
    {
      VIP_ENCDOUBLE spread = ((VIP_ENCDOUBLE)DAMPING * ranks[src]) / (VIP_ENCDOUBLE)NUM_NODES;
      for (unsigned dst = 0; dst < NUM_NODES; ++dst)
        next_ranks[dst] = next_ranks[dst] + spread;
      continue;
    }

    VIP_ENCDOUBLE push = ((VIP_ENCDOUBLE)DAMPING * ranks[src]) / (VIP_ENCDOUBLE)out_degree[src];
    for (unsigned dst = 0; dst < NUM_NODES; ++dst)
    {
      VIP_ENCBOOL has_edge = ((VIP_ENCUINT64)adj[src][dst]) == (VIP_ENCUINT64)1;
      next_ranks[dst] = next_ranks[dst] + cmov(has_edge, push, (VIP_ENCDOUBLE)0.0);
    }
  }
}

static VIP_ENCBOOL
commit_and_check_convergence(void)
{
  VIP_ENCBOOL changed = 0;

  for (unsigned i = 0; i < NUM_NODES; ++i)
  {
    VIP_ENCDOUBLE diff = myfabs(next_ranks[i] - ranks[i]);
    changed = changed || (diff > (VIP_ENCDOUBLE)THRESHOLD);
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
    VIP_ENCBOOL changed = commit_and_check_convergence();
    ++iter;
    if (!changed.decrypt())
      break;
  }

  VIP_ENCDOUBLE sum = 0.0;
  for (unsigned i = 0; i < NUM_NODES; ++i)
    sum = sum + ranks[i];

  libmin_printf("pagerank iterations: %u\n", iter);
  libmin_printf("pagerank sum: %.9f\n", sum.decrypt());
  for (unsigned i = 0; i < NUM_NODES; ++i)
  {
    VIP_ENCDOUBLE norm = ranks[i] / sum;
    libmin_printf("node %u rank %.9f\n", i, norm.decrypt());
  }

  libmin_success();
  return 0;
}
