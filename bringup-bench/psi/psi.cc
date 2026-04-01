#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"

#define MAX_SETVAL    5000
#define SETA_SIZE     500
#define SETB_SIZE     1000

void
set_init(int64e_t *set, size_t set_size)
{
  int64_t _set[set_size];

  for (unsigned i=0; i < set_size; i++)
  {
  redo:
    int val = libmin_rand() % MAX_SETVAL;
    for (unsigned j=0; i != 0 && j < i; j++)
    {
      if (_set[j] == val)
        goto redo;
    }
    set[i] = _set[i] = val;
  }
}

void
set_print(const char *name, int64e_t *set, size_t set_size)
{
  libmin_printf("%s:\n", name);
  for (unsigned i=0; i < set_size; i++)
    libmin_printf("  %s[%u] = %ld\n", name, i, set[i].decrypt());
}

void
set_intersect(int64e_t *setA, size_t seta_size, int64e_t *setB, size_t setb_size, int64e_t *setA_match)
{
  for (unsigned i=0; i < seta_size; i++)
  {
    int64e_t match = /* false */0;
    for (unsigned j=0; j < setb_size; j++)
      match = cmov(setA[i] == setB[j], /* true */1, match);
    setA_match[i] = match;
  }
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

  int64e_t setA[SETA_SIZE];
  int64e_t setB[SETB_SIZE];
  int64e_t setA_match[SETA_SIZE];

  // initialize the set vectors
  set_init(setA, SETA_SIZE); 
  set_print("setA", setA, SETA_SIZE);
  set_init(setB, SETB_SIZE); 
  set_print("setB", setB, SETB_SIZE);

  {
    // Stopwatch s("VIP_Bench Runtime");
    set_intersect(setA, SETA_SIZE, setB, SETB_SIZE, setA_match);
  }

  // print the intersection
  libmin_printf("setA & setB:\n");
  
  unsigned j=0;
  for (unsigned i=0; i < SETA_SIZE; i++)
  {
    if ((setA_match[i]).decrypt())
      libmin_printf("  setA_and_setB[%3u] = %ld\n", j++, setA[i].decrypt());
  }
  libmin_printf("INFO: cardinality(setA & setB) == %u\n", j);
 
  libmin_success();
  return 0;
}
