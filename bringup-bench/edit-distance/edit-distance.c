#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"
uint128_t simon_key = SIMON128_KEY;
simon_state_t simon_state;

typedef mojov_mem_fast_u64_t _uint64e_t;
typedef mojov_mem_fast_fp64_t _fp64e_t;
#include "mojov-exo.h"

// import test genetics data
#include "gene-data.h"

// total tests to run
#define N_TESTS 4

_uint64e_t
min3(_uint64e_t x, _uint64e_t y, _uint64e_t z)
{
  return _cmov(_land(_slt(x, y), _slt(x, z)), x, _cmov(_slt(y, z), y, z));
}

_uint64e_t
EditDistance(_uint64e_t *str1, _uint64e_t *str2)
{
  unsigned i, j;
  _uint64e_t edit_matrix[GENE_LEN+1][GENE_LEN+1];

  for (i = 0; i < GENE_LEN+1; i++)
    edit_matrix[i][0] = _enc(i);

  for (j = 0; j < GENE_LEN+1; j++)
    edit_matrix[0][j] = _enc(j);

  for (i = 0; i < GENE_LEN; i++ )
  {
    for (j = 0; j < GENE_LEN; j++ )
    {
      _uint64e_t edit = _cmov(_seq(str1[i], str2[j]), _enc(0u), _enc(1u));
      edit_matrix[i + 1][j + 1] = min3(_addi(edit_matrix[i][j+1], 1), _addi(edit_matrix[i+1][j], 1), _add(edit_matrix[i][j], edit));
    }
  }
  return edit_matrix[GENE_LEN][GENE_LEN];
}


int
main(void)
{
  if (mojov_configure_kmsm_from_dc_fast() != 0)
    return -1;

  // initilize cipher engine, for checking results
  simon_128_128_keyexpand(&simon_state, simon_key, 68);

  //
  // mprivregcfg tests
  //
  libmin_printf("** Running CSR[privreg] tests...\n");

  uint64_t val;

  // read reset value
  val = mojov_read_mprivregcfg();
  libmin_printf("Initial mprivregcfg = 0x%lx, ", val);
  mojov_print_mprivregcfg(val);
  libmin_printf("\n");

  // enable private register semantics (bit 0 = 1)
  if (mojov_enable_and_verify() != 0)
    return -1;

  val = mojov_read_mprivregcfg();
  libmin_printf("After enable, mprivregcfg = 0x%lx, ", val);
  mojov_print_mprivregcfg(val);
  libmin_printf("\n");

  // initialize the pseudo-RNG
  libmin_srand(42);

  libmin_printf("------Within the randoms array------\n");

  // encrypt test genetics data
  _uint64e_t gene_enc[N_GENE_DATA][GENE_LEN];
  for (unsigned i=0; i < N_GENE_DATA; i++)
  {
    for (unsigned j=0; j < GENE_LEN; j++)
    {
      gene_enc[i][j] = _enc((uint64_t)gene_data[i][j]);
    }
  }

  for (unsigned i=0; i < N_TESTS; i++)
  {
    libmin_printf("++ compute distance from `%s' ++\n", gene_data[i]);
    for (unsigned j=0; j < N_GENE_DATA; j++)
    {
      _uint64e_t ed = EditDistance(gene_enc[i], gene_enc[j]);
      libmin_printf("  edit_distance(%s, %s) == %lu\n", gene_data[i], gene_data[j], mojov_decrypt_fast_u64(&simon_state, ed, CONTRACT_SIG));
    }
    libmin_printf("\n");
  }

  libmin_success();
  return 0;
}

