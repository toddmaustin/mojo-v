#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

typedef mojov_mem_fast_u64_t _uint64e_t;
typedef mojov_mem_fast_fp64_t _fp64e_t;
#include "mojov-exo.h"

// import test genetics data
#include "gene-data.h"

// total tests to run
#define N_TESTS 4

uint64e_t
min3(uint64e_t x, uint64e_t y, uint64e_t z)
{
  return cmov((x < y) && (x < z), x, cmov(y < z, y, z));
}

uint64e_t
EditDistance(uint64e_t *str1, uint64e_t *str2)
{
  unsigned i, j;
  uint64e_t edit_matrix[GENE_LEN+1][GENE_LEN+1];

  for (i = 0; i < GENE_LEN+1; i++)
    edit_matrix[i][0] = i;

  for (j = 0; j < GENE_LEN+1; j++)
    edit_matrix[0][j] = j;

  for (i = 0; i < GENE_LEN; i++ )
  {
    for (j = 0; j < GENE_LEN; j++ )
    {
      uint64e_t edit = cmov(str1[i] == str2[j], 0lu, 1lu);
      edit_matrix[i + 1][j + 1] = min3(edit_matrix[i][j+1] + 1, edit_matrix[i+1][j] + 1, edit_matrix[i][j] + edit);
    }
  }
  return edit_matrix[GENE_LEN][GENE_LEN];
}


int
main(void)
{
  if (mojov_configure_kmsm_from_dc_fast() != 0)
    return -1;

  // enable private register semantics (bit 0 = 1)
  if (mojov_enable_and_verify() != 0)
    return -1;

  // enable encrypted variable debugging
  if (debug_context(SIMON128_KEY, CONTRACT_SIG) != 0)
    return -1;

  // initialize the pseudo-RNG
  libmin_srand(42);

  libmin_printf("------Within the randoms array------\n");

  // encrypt test genetics data
  uint64e_t gene_enc[N_GENE_DATA][GENE_LEN];
  for (unsigned i=0; i < N_GENE_DATA; i++)
  {
    for (unsigned j=0; j < GENE_LEN; j++)
      gene_enc[i][j] = gene_data[i][j];
  }

  for (unsigned i=0; i < N_TESTS; i++)
  {
    libmin_printf("++ compute distance from `%s' ++\n", gene_data[i]);
    for (unsigned j=0; j < N_GENE_DATA; j++)
    {
      uint64e_t ed = EditDistance(gene_enc[i], gene_enc[j]);
      libmin_printf("  edit_distance(%s, %s) == %lu\n", gene_data[i], gene_data[j], ed.decrypt());
    }
    libmin_printf("\n");
  }

  libmin_success();
  return 0;
}

