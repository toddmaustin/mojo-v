#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"
uint128_t simon_key = SIMON128_KEY;
simon_state_t simon_state;

typedef mojov_mem_fast_u64_t _uint64e_t;
typedef mojov_mem_fast_fp64_t _fp64e_t;
#include "mojov-exo.h"

// debug
#define _DEC(X)   (mojov_decrypt_fast_u64(&simon_state, (X), CONTRACT_SIG))

#define N 50
#define W 250

// FIXME: uint64e_t array initializers are NOT working as yet
uint64e_t values[N];
uint64_t _values[N] = { 27, 34, 9, 22, 8, 17, 22, 21, 23, 19, 7, 36, 11, 42, 37, 16, 10, 26, 10, 50, 23, 46, 37, 3, 14, 16, 35, 14, 15, 44, 49, 2, 45, 3, 15, 1, 34, 44, 19, 25, 43, 28, 26, 4, 30, 24, 49, 11, 48, 13 };

uint64e_t weights[N];
uint64_t _weights[N] = { 23, 47, 22, 15, 42, 30, 15, 32, 47, 33, 15, 38, 44, 7, 16, 34, 30, 33, 3, 2, 43, 31, 46, 17, 30, 1, 34, 21, 30, 21, 29, 21, 36, 14, 18, 21, 13, 3, 27, 44, 33, 11, 9, 31, 40, 40, 30, 9, 41, 31 };

// A utility function that returns maximum of two integers
static inline uint64e_t
max(const uint64e_t a, const uint64e_t b)
{
  return cmov(a > b, a, b);
}

// Returns the maximum value that can be put in a knapsack of capacity W
void
knapSack(uint64e_t wt[], uint64e_t val[], uint64e_t K[N+1][W+1])
{
  int i, w;

  // Build table K[][] in bottom up manner
  for (i = 0; i <= N; i++)
  {
    for (w = 0; w <= W; w++)
    {
      if (i==0 || w==0)
        K[i][w] = 0;
      else
      {
        uint64e_t Kw = -1;
        for (int k=0; k<=W; k++)
          Kw = cmov((uint64e_t)k == ((uint64e_t)w-wt[i-1]), K[i-1][k], Kw);
        uint64e_t maxval = max(val[i-1] + Kw,  K[i-1][w]);
        K[i][w] = cmov(wt[i-1] <= w, maxval, K[i-1][w]);
      }
    }
  }
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

  // initialize inputs
  for (unsigned i=0; i<N; i++)
  {
    values[i] = _values[i];
    weights[i] = _weights[i];
  }

  int n = N;
  int w = W;
  uint64e_t K[N+1][W+1];

  {
    // performance monitoring
    // uint64_t icnt_start = __instret();

    knapSack(weights, values, K);

    // uint64_t icnt_end = __instret();
    // libmin_printf("INFO: bubblesort inst count = %lu.\n", icnt_end - icnt_start + 1);
  }

	libmin_printf("Max value: %d\n", mojov_decrypt_fast_u64(&simon_state, K[n][W], CONTRACT_SIG));
	
  libmin_printf("Selected packs:\n");
  while (n != 0)
  {
    if (mojov_decrypt_fast_u64(&simon_state, K[n][w], CONTRACT_SIG) != mojov_decrypt_fast_u64(&simon_state, K[n - 1][w], CONTRACT_SIG)) {
      libmin_printf("  Package %d with wt=%d and val=%d\n",
                    n, mojov_decrypt_fast_u64(&simon_state, weights[n - 1], CONTRACT_SIG), mojov_decrypt_fast_u64(&simon_state, values[n - 1], CONTRACT_SIG));
      w = w - mojov_decrypt_fast_u64(&simon_state, weights[n-1], CONTRACT_SIG);
    }
    n--;
  }
  libmin_printf("Total weight: %d\n", W - w);

  libmin_success();
  return 0;
}

