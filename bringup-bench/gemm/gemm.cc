#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"
uint128_t simon_key = SIMON128_KEY;
simon_state_t simon_state;

typedef mojov_mem_fast_u64_t _uint64e_t;
typedef mojov_mem_fast_fp64_t _fp64e_t;
#include "mojov-exo.h"

// debug support
#define _DEC_U64(X)   (mojov_decrypt_fast_u64(&simon_state, (X), CONTRACT_SIG))
#define _DEC_FP64(X)  (mojov_decrypt_fast_fp64(&simon_state, (X), CONTRACT_SIG))

#define M 32
#define N 32
#define K 32

#define I_ALPHA 3
#define I_BETA  2

#define D_ALPHA 1.5
#define D_BETA  0.5

/* integer GEMM state */
static uint64e_t IA[M][K];
static int64_t IA_ref[M][K];
static uint64e_t IB[K][N];
static int64_t IB_ref[K][N];
static uint64e_t IC[M][N];
static int64_t IC_ref[M][N];

/* double GEMM state */
static fp64e_t DA[M][K];
static double DA_ref[M][K];
static fp64e_t DB[K][N];
static double DB_ref[K][N];
static fp64e_t DC[M][N];
static double DC_ref[M][N];

static void
init_int_matrices(void)
{
  for (int i = 0; i < M; i++)
  {
    for (int k = 0; k < K; k++)
    {
      int64_t init = (int64_t)(((i * 17 + k * 13 + 1) % 31) - 15);
      IA[i][k] = init;
      IA_ref[i][k] = init;
    }
  }

  for (int k = 0; k < K; k++)
  {
    for (int j = 0; j < N; j++)
    {
      int64_t init = (int64_t)(((k * 7 + j * 19 + 3) % 29) - 14);
      IB[k][j] = init;
      IB_ref[k][j] = init;
    }
  }

  for (int i = 0; i < M; i++)
  {
    for (int j = 0; j < N; j++)
    {
      int64_t init = (int64_t)(((i * 5 + j * 11 + 7) % 23) - 11);
      IC[i][j] = init;
      IC_ref[i][j] = init;
    }
  }
}

static void
int_gemm_kernel(void)
{
  for (int i = 0; i < M; i++)
  {
    for (int j = 0; j < N; j++)
    {
      uint64e_t acc = 0;

      for (int k = 0; k < K; k++)
        acc += IA[i][k] * IB[k][j];

      IC[i][j] = ((int64_t)I_ALPHA * acc) + ((int64_t)I_BETA * IC[i][j]);
    }
  }
}

static void
int_gemm_reference(void)
{
  for (int i = 0; i < M; i++)
  {
    for (int j = 0; j < N; j++)
      IC_ref[i][j] *= (int64_t)I_BETA;
  }

  for (int i = 0; i < M; i++)
  {
    for (int k = 0; k < K; k++)
    {
      int64_t aik = (int64_t)I_ALPHA * IA_ref[i][k];

      for (int j = 0; j < N; j++)
        IC_ref[i][j] += aik * IB_ref[k][j];
    }
  }
}

static uint64e_t
int_checksum(void)
{
  uint64e_t checksum = 0;
  const uint64_t mod = 1000000007ul;

  for (int i = 0; i < M; i++)
  {
    for (int j = 0; j < N; j++)
    {
      uint64e_t term = IC[i][j] + 100000l;
      checksum = ((checksum * 131ul) + term) % mod;
    }
  }

  return checksum;
}

static int
run_int_gemm(void)
{
  init_int_matrices();

  libtarg_start_perf();
  int_gemm_kernel();
  libtarg_stop_perf();

  int_gemm_reference();

  for (int i = 0; i < M; i++)
  {
    for (int j = 0; j < N; j++)
    {
      if (_DEC_U64(IC[i][j]) != (uint64_t)IC_ref[i][j])
      {
        libmin_printf("ERROR: int GEMM mismatch at (%d, %d), got %ld expected %ld\n",
                      i, j, _DEC_U64(IC[i][j]), IC_ref[i][j]);
        return 1;
      }
    }
  }

  libmin_printf("INFO: int64 GEMM verified, checksum=0x%08lx\n", _DEC_U64(int_checksum()));
  return 0;
}

static void
init_fp_matrices(void)
{
  /*
   * These are chosen to be exact binary fractions, so the reference
   * and main kernel should match exactly even with a different loop order.
   */
  for (int i = 0; i < M; i++)
  {
    for (int k = 0; k < K; k++)
    {
      double init = (double)(((i * 9 + k * 5 + 1) % 17) - 8) / 8.0;
      DA[i][k] = init;
      DA_ref[i][k] = init;
    }
  }

  for (int k = 0; k < K; k++)
  {
    for (int j = 0; j < N; j++)
    {
      double init = (double)(((k * 3 + j * 7 + 2) % 19) - 9) / 16.0;
      DB[k][j] = init;
      DB_ref[k][j] = init;
    }
  }

  for (int i = 0; i < M; i++)
  {
    for (int j = 0; j < N; j++)
    {
      double init = (double)(((i * 4 + j * 6 + 3) % 13) - 6) / 4.0;
      DC[i][j] = init;
      DC_ref[i][j] = init;
    }
  }
}

static void
double_gemm_kernel(void)
{
  for (int i = 0; i < M; i++)
  {
    for (int j = 0; j < N; j++)
    {
      fp64e_t acc = 0.0;

      for (int k = 0; k < K; k++)
        acc += DA[i][k] * DB[k][j];

      DC[i][j] = D_ALPHA * acc + D_BETA * DC[i][j];
    }
  }
}

static void
double_gemm_reference(void)
{
  for (int i = 0; i < M; i++)
  {
    for (int j = 0; j < N; j++)
      DC_ref[i][j] *= D_BETA;
  }

  for (int i = 0; i < M; i++)
  {
    for (int k = 0; k < K; k++)
    {
      double aik = D_ALPHA * DA_ref[i][k];

      for (int j = 0; j < N; j++)
        DC_ref[i][j] += aik * DB_ref[k][j];
    }
  }
}

#if 0
static uint64e_t
double_checksum(void)
{
  uint64e_t checksum = 0;
  const uint64_t mod = 1000000007ul;

  /*
   * All results are exact multiples of 1/256, so scale and hash
   * as integers for a stable printed checksum.
   */
  for (int i = 0; i < M; i++)
  {
    for (int j = 0; j < N; j++)
    {
      uint64e_t scaled = DC[i][j] * 256.0;
      uint64e_t term = scaled + 100000l;
      checksum = ((checksum * 131ul) + term) % mod;
    }
  }

  return checksum;
}
#endif

static int
run_fp_gemm(void)
{
  init_fp_matrices();

  libtarg_start_perf();
  double_gemm_kernel();
  libtarg_stop_perf();

  double_gemm_reference();

  for (int i = 0; i < M; i++)
  {
    for (int j = 0; j < N; j++)
    {
      if (_DEC_FP64(DC[i][j]) != DC_ref[i][j])
      {
        libmin_printf(
          "ERROR: fp64 GEMM mismatch at (%d, %d)\n",
          i, j);
        return 1;
      }
    }
  }

  // libmin_printf("INFO: fp64 GEMM verified,  checksum=0x%08lx\n", _DEC_U64(fp_checksum()));
  return 0;
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
  libmin_srand(142);

  if (run_int_gemm())
  {
    libmin_fail(1);
    return 1;
  }

  if (run_fp_gemm())
  {
    libmin_fail(1);
    return 1;
  }

  libmin_printf("INFO: all GEMM tests passed\n");
  libmin_success();
  return 0;
}
