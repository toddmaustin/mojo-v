#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"
uint128_t simon_key = SIMON128_KEY;
simon_state_t simon_state;

typedef mojov_mem_fast_u64_t _uint64e_t;
typedef mojov_mem_fast_fp64_t _fp64e_t;
#include "mojov-exo.h"

// debug interfaces
#define _DEC_U64(X)   (mojov_decrypt_fast_u64(&simon_state, (X), CONTRACT_SIG))
#define _DEC_I64(X)   (mojov_decrypt_fast_i64(&simon_state, (X), CONTRACT_SIG))
#define _DEC_FP64(X)  (mojov_decrypt_fast_fp64(&simon_state, (X), CONTRACT_SIG))

/* Period parameters */  
#define N 624
#define M 397

uint64e_t MATRIX_A;     /* constant vector a */
uint64e_t UPPER_MASK;   /* most significant w-r bits */
uint64e_t LOWER_MASK;   /* least significant r bits */

/* Tempering parameters */   
uint64e_t TEMPERING_MASK_B;
uint64e_t TEMPERING_MASK_C;
uint64e_t TEMPERING_SHIFT_U;
uint64e_t TEMPERING_SHIFT_S;
uint64e_t TEMPERING_SHIFT_T;
uint64e_t TEMPERING_SHIFT_L;

static uint64e_t mt[N];   /* the array for the state vector  */

/* USED FOR ARRAY ACCESS, NOT SECRET */
static int         mti=N+1; /* mti==N+1 means mt[N] is not initialized */


/* Initializing the array with a seed */
void
sgenrand(uint64e_t seed)
{
  int i;

  for (i=0;i<N;i++)
    {
      mt[i] = seed & 0xffff0000;
      seed = (uint64e_t)69069 * seed + 1;
      mt[i] = mt[i] | ((seed & 0xffff0000) >> 16);
      seed = (uint64e_t)69069 * seed + 1;
    }
  mti = N;
}


uint64e_t 
genrand(void)
{
  uint64e_t y;

  if (mti >= N) { /* generate N words at one time */
    int kk;

    if (mti == N+1)   /* if sgenrand() has not been called, */
    {
      libmin_printf("ERROR: sgenrand() has not been called.\n");
      libmin_fail(1);
    }

    for (kk=0;kk<N-M;kk++){
	      y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
	      mt[kk] = mt[kk+M] ^ (y >> 1) ^ ((y & 0x1)*MATRIX_A);
    }
    for (;kk<N-1;kk++){
	      y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
	      mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ ((y & 0x1)*MATRIX_A); 
    }

    y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
    mt[N-1] = mt[M-1] ^ (y >> 1) ^ ((y & 0x1)*MATRIX_A);

    mti = 0;
  }
  
  y = mt[mti++];
  y = y ^ (y >> TEMPERING_SHIFT_U);
  y = y ^ ((y << TEMPERING_SHIFT_S) & TEMPERING_MASK_B);
  y = y ^ ((y << TEMPERING_SHIFT_T) & TEMPERING_MASK_C);
  y = y ^ (y >> TEMPERING_SHIFT_L);

  return y; 
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

  MATRIX_A   = 0x9908b0df;   /* constant vector a */
  UPPER_MASK = 0x80000000;   /* most significant w-r bits */
  LOWER_MASK = 0x7fffffff;   /* least significant r bits */
  
  /* Tempering parameters */   
  TEMPERING_MASK_B  = 0x9d2c5680;
  TEMPERING_MASK_C  = 0xefc60000;
  TEMPERING_SHIFT_U = 11;
  TEMPERING_SHIFT_S = 7;
  TEMPERING_SHIFT_T = 15;
  TEMPERING_SHIFT_L = 18;

  int steps = 1000;
  int i, j;
  
  {
    // Stopwatch s("VIP_Bench Runtime");
    uint64e_t seedval = 42;
    sgenrand(seedval);

    uint64e_t randval;
    for (i=0,j=0; i<steps; i++)
    {
      randval = genrand();
      libmin_printf("%10u, ", _DEC_U64(randval));
      if (++j%8==0)
        libmin_printf("\n");
    }
  }


  libmin_success();
  return 0;
}
