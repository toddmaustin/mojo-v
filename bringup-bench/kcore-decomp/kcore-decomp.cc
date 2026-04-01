#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

typedef mojov_mem_fast_u64_t _uint64e_t;
typedef mojov_mem_fast_fp64_t _fp64e_t;
#include "mojov-exo.h"

#define NVERTS 8
#define MAX_DEG 4
#define MAX_K NVERTS

// Secret adjacency-list dataset for an undirected sparse graph.
// Vertex labels are 0..7.
static const uint64_t graph_neighbors[NVERTS][MAX_DEG] = {
  {1, 2, 3, 0},
  {0, 2, 3, 4},
  {0, 1, 3, 5},
  {0, 1, 2, 6},
  {1, 5, 7, 0},
  {2, 4, 7, 0},
  {3, 7, 0, 0},
  {4, 5, 6, 0}
};

static const uint64_t graph_valid[NVERTS][MAX_DEG] = {
  {1, 1, 1, 0},
  {1, 1, 1, 1},
  {1, 1, 1, 1},
  {1, 1, 1, 1},
  {1, 1, 1, 0},
  {1, 1, 1, 0},
  {1, 1, 0, 0},
  {1, 1, 1, 0}
};

static uint64e_t secret_neighbors[NVERTS][MAX_DEG];
static uint64e_t secret_valid[NVERTS][MAX_DEG];

static void
init_secret_dataset(void)
{
  for (unsigned v = 0; v < NVERTS; ++v)
  {
    for (unsigned s = 0; s < MAX_DEG; ++s)
    {
      secret_neighbors[v][s] = graph_neighbors[v][s];
      secret_valid[v][s] = graph_valid[v][s];
    }
  }
}

// Compute k-core labels with fixed-structure, data-oblivious loops.
static void
kcore_decompose(uint64e_t *core_label)
{
  uint64e_t remaining[NVERTS];

  for (unsigned v = 0; v < NVERTS; ++v)
  {
    remaining[v] = 1;
    core_label[v] = 0;
  }

  for (unsigned k = 1; k <= MAX_K; ++k)
  {
    for (unsigned pass = 0; pass < NVERTS; ++pass)
    {
      uint64e_t next_remaining[NVERTS];

      for (unsigned v = 0; v < NVERTS; ++v)
      {
        uint64e_t degree = 0;

        for (unsigned s = 0; s < MAX_DEG; ++s)
        {
          uint64e_t nb = secret_neighbors[v][s];
          uint64e_t nb_active = 0;

          for (unsigned u = 0; u < NVERTS; ++u)
          {
            uint64e_t is_nb = (nb == u);
            nb_active = cmov(is_nb, remaining[u], nb_active);
          }

          degree = degree + cmov(secret_valid[v][s], nb_active, 0);
        }

        uint64e_t peel = remaining[v] && (degree < k);
        next_remaining[v] = cmov(peel, 0, remaining[v]);
        core_label[v] = cmov(peel, k - 1, core_label[v]);
      }

      for (unsigned v = 0; v < NVERTS; ++v)
        remaining[v] = next_remaining[v];
    }
  }

  for (unsigned v = 0; v < NVERTS; ++v)
    core_label[v] = cmov(remaining[v], MAX_K, core_label[v]);
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

  init_secret_dataset();

  uint64e_t core_label[NVERTS];
  kcore_decompose(core_label);

  libmin_printf("kcore-decomp coreness by vertex:\n");
  for (unsigned v = 0; v < NVERTS; ++v)
    libmin_printf("  vertex %u -> k = %lu\n", v, core_label[v].decrypt());

  libmin_success();
  return 0;
}
