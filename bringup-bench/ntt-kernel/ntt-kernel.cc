#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

typedef mojov_mem_fast_u64_t _uint64e_t;
typedef mojov_mem_fast_fp64_t _fp64e_t;
#include "mojov-exo.h"

#define N 256
#define MOD 998244353l
#define PRIMITIVE_ROOT 3l

static int64e_t a[N];
static int64_t a_ref[N];
static int64e_t b[N];
static int64_t b_ref[N];
static int64e_t fa[N];
static int64e_t fb[N];
static int64e_t conv[N];
static int64_t ref[N];

static int64e_t
mod_add(int64e_t x, int64e_t y)
{
  x += y;
  int64e_t pred = (x >= MOD);
  x = cmov(pred, x - MOD, x);
  return x;
}

static int64e_t
mod_sub(int64e_t x, int64e_t y)
{
  x -= y;
  int64e_t pred = (x < 0);
  x = cmov(pred, x + MOD, x);
  return x;
}

static int64e_t
mod_mul(int64e_t x, int64e_t y)
{
  return (x * y) % MOD;
}

static int64e_t
mod_pow(int64e_t base, int64_t exp)
{
  int64e_t result = 1;

  while (exp > 0)
  {
    if (exp & 1)
      result = mod_mul(result, base);
    base = mod_mul(base, base);
    exp >>= 1;
  }

  return result;
}

static void
bit_reverse_permute(int64e_t *data)
{
  int j = 0;

  for (int i = 1; i < N; i++)
  {
    int bit = N >> 1;

    while (j & bit)
    {
      j ^= bit;
      bit >>= 1;
    }

    j ^= bit;

    if (i < j)
    {
      int64e_t tmp = data[i];
      data[i] = data[j];
      data[j] = tmp;
    }
  }
}

static void
ntt(int64e_t *data, int invert)
{
  bit_reverse_permute(data);

  for (int len = 2; len <= N; len <<= 1)
  {
    int64e_t wlen = mod_pow(PRIMITIVE_ROOT, (MOD - 1) / len);

    if (invert)
      wlen = mod_pow(wlen, MOD - 2);

    for (int i = 0; i < N; i += len)
    {
      int64e_t w = 1;

      for (int j = 0; j < len / 2; j++)
      {
        int64e_t u = data[i + j];
        int64e_t v = mod_mul(data[i + j + len / 2], w);

        data[i + j] = mod_add(u, v);
        data[i + j + len / 2] = mod_sub(u, v);

        w = mod_mul(w, wlen);
      }
    }
  }

  if (invert)
  {
    int64e_t inv_n = mod_pow(N, MOD - 2);

    for (int i = 0; i < N; i++)
      data[i] = mod_mul(data[i], inv_n);
  }
}

static void
init_data(void)
{
  for (int i = 0; i < N; i++)
  {
    a[i] = a_ref[i] = (int64_t)((i * 17 + 5) % 257);
    b[i] = b_ref[i] = (int64_t)(((i * i * 7) + (i * 3) + 11) % 257);

    fa[i] = a[i];
    fb[i] = b[i];
    conv[i] = 0;
    ref[i] = 0;
  }
}

static void
ntt_convolution_kernel(void)
{
  ntt(fa, 0);
  ntt(fb, 0);

  for (int i = 0; i < N; i++)
    conv[i] = mod_mul(fa[i], fb[i]);

  ntt(conv, 1);
}

static void
reference_cyclic_convolution(void)
{
  for (int i = 0; i < N; i++)
  {
    for (int j = 0; j < N; j++)
    {
      int idx = (i + j) % N;
      ref[idx] += a_ref[i] * b_ref[j];
      ref[idx] %= MOD;
    }
  }
}

static uint64e_t
compute_checksum(void)
{
  uint64e_t checksum = 0;
  const uint64_t mod = 1000000007ul;

  for (int i = 0; i < N; i++)
    checksum = ((checksum * 131ul) + (uint64e_t)conv[i]) % mod;

  return checksum;
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

  init_data();

  libtarg_start_perf();
  ntt_convolution_kernel();
  libtarg_stop_perf();

  reference_cyclic_convolution();

  for (int i = 0; i < N; i++)
  {
    if ((conv[i] != ref[i]).decrypt())
    {
      libmin_printf("ERROR: NTT mismatch at %d, got %ld expected %ld\n", i, conv[i].decrypt(), ref[i]);
      libmin_fail(1);
      return 1;
    }
  }

  libmin_printf("INFO: NTT convolution verified, checksum=0x%08lx\n", compute_checksum().decrypt());

  libmin_success();
  return 0;
}
