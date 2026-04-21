/*
 * verlet.cc
 *
 * Minimal Verlet integration benchmark (2D harmonic oscillators), adapted for
 * Mojo-V. Data initialized by vb_init() is stored in encrypted types.
 */

#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"

#ifndef VB_N
#define VB_N 128
#endif
#ifndef VB_STEPS
#define VB_STEPS 50
#endif
#ifndef VB_DT
#define VB_DT 1.0e-3
#endif
#ifndef VB_K
#define VB_K 5.0
#endif
#ifndef VB_DIM
#define VB_DIM 2
#endif

#define VB_FP fp64e_t

static inline uint32_t
vb_lcg(uint32_t *state)
{
  *state = (*state) * 1664525u + 1013904223u;
  return *state;
}

static inline double
vb_urand(uint32_t *state)
{
  return (double)(vb_lcg(state) & 0x00FFFFFFu) / 16777216.0;
}

/* vb_init() writes only encrypted state. */
static VB_FP x[VB_N * VB_DIM];
static VB_FP v[VB_N * VB_DIM];
static VB_FP a[VB_N * VB_DIM];

static void
vb_init(void)
{
  uint32_t rng = 0x12345678u;

  for (int i = 0; i < VB_N; ++i)
  {
    for (int d = 0; d < VB_DIM; ++d)
    {
      double px = vb_urand(&rng) * 2.0 - 1.0;
      double pv = vb_urand(&rng) * 2.0 - 1.0;
      int idx = i * VB_DIM + d;

      x[idx] = px;
      v[idx] = pv * 0.1;
      a[idx] = (-VB_K) * px;
    }
  }
}

static VB_FP a_old[VB_N * VB_DIM];

static void
vb_step_avg(double dt)
{
  const double half_dt2 = 0.5 * dt * dt;

  for (int i = 0; i < VB_N * VB_DIM; ++i)
    a_old[i] = a[i];

  for (int i = 0; i < VB_N; ++i)
    for (int d = 0; d < VB_DIM; ++d)
    {
      int idx = i * VB_DIM + d;
      x[idx] = x[idx] + v[idx] * dt + a_old[idx] * half_dt2;
    }

  for (int i = 0; i < VB_N; ++i)
    for (int d = 0; d < VB_DIM; ++d)
    {
      int idx = i * VB_DIM + d;
      a[idx] = (-VB_K) * x[idx];
    }

  for (int i = 0; i < VB_N; ++i)
    for (int d = 0; d < VB_DIM; ++d)
    {
      int idx = i * VB_DIM + d;
      v[idx] = v[idx] + ((a_old[idx] + a[idx]) * 0.5) * dt;
    }
}

static uint64_t
vb_checksum(void)
{
  const double scale = 1.0e6;
  uint64_t h = 0xcbf29ce484222325ULL;

  for (int i = 0; i < VB_N * VB_DIM; ++i)
  {
    int64_t xi = (int64_t)(x[i].decrypt() * scale);
    int64_t vi = (int64_t)(v[i].decrypt() * scale);
    uint64_t ux = (uint64_t)xi;
    uint64_t uv = (uint64_t)vi;

    h ^= ux + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
    h ^= uv + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
  }

  return h;
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

  vb_init();

  for (int s = 0; s < VB_STEPS; ++s)
    vb_step_avg(VB_DT);

  uint64_t sum = vb_checksum();

  libmin_printf("verlet2d: N=%d steps=%d dt=%f k=%f dim=%d fp=fp64e\n", VB_N, VB_STEPS, VB_DT, VB_K, VB_DIM);
  libmin_printf("checksum=0x%08x%08x\n", (uint32_t)(sum >> 32), (uint32_t)(sum & 0xFFFFFFFFu));

  libmin_success();
  return 0;
}
