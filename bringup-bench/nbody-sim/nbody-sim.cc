#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"

#define BODIES 20u
#define STEPS 80u
#define DT 0.01
#define GCONST 1.0
#define SOFTENING 0.0005

#define BLOOM_BITS 512u
#define BLOOM_HASHES 3u

static fp64e_t pos_x[BODIES], pos_y[BODIES], pos_z[BODIES];
static fp64e_t vel_x[BODIES], vel_y[BODIES], vel_z[BODIES];
static fp64e_t mass[BODIES];

// Bloom filter contents are encrypted in memory.
static uint64e_t encounter_bloom[BLOOM_BITS];

static inline uint64_t
mix64(uint64_t x)
{
  x ^= x >> 33;
  x *= 0xff51afd7ed558ccdULL;
  x ^= x >> 33;
  x *= 0xc4ceb9fe1a85ec53ULL;
  x ^= x >> 33;
  return x;
}

static inline uint64_t
bloom_index(uint64_t pair_tag, unsigned which)
{
  return mix64(pair_tag + (uint64_t)(0x9e3779b97f4a7c15ULL * (which + 1))) % BLOOM_BITS;
}

static void
bloom_record(uint64_t pair_tag)
{
  for (unsigned i = 0; i < BLOOM_HASHES; ++i)
  {
    uint64_t idx = bloom_index(pair_tag, i);
    encounter_bloom[idx] = 1;
  }
}

static void
init_system(void)
{
  for (unsigned i = 0; i < BODIES; ++i)
  {
    double fi = (double)i;
    pos_x[i] = (fi - 10.0) * 0.07;
    pos_y[i] = (fi - 7.0) * 0.05;
    pos_z[i] = (fi - 4.0) * 0.03;

    vel_x[i] = 0.002 * (double)((i % 5) - 2);
    vel_y[i] = 0.0015 * (double)((i % 7) - 3);
    vel_z[i] = 0.0012 * (double)((i % 3) - 1);

    mass[i] = 0.8 + 0.03 * fi;
  }

  for (unsigned i = 0; i < BLOOM_BITS; ++i)
    encounter_bloom[i] = 0;
}

static void
simulate_step(unsigned step)
{
  fp64e_t acc_x[BODIES];
  fp64e_t acc_y[BODIES];
  fp64e_t acc_z[BODIES];

  for (unsigned i = 0; i < BODIES; ++i)
  {
    acc_x[i] = 0;
    acc_y[i] = 0;
    acc_z[i] = 0;
  }

  for (unsigned i = 0; i < BODIES; ++i)
  {
    for (unsigned j = i + 1; j < BODIES; ++j)
    {
      fp64e_t dx = pos_x[j] - pos_x[i];
      fp64e_t dy = pos_y[j] - pos_y[i];
      fp64e_t dz = pos_z[j] - pos_z[i];

      fp64e_t dist2 = dx * dx + dy * dy + dz * dz + SOFTENING;
      fp64e_t inv_dist = 1.0 / libmin_sqrt(dist2);
      fp64e_t inv_dist3 = inv_dist * inv_dist * inv_dist;
      fp64e_t scale_i = GCONST * mass[j] * inv_dist3;
      fp64e_t scale_j = GCONST * mass[i] * inv_dist3;

      acc_x[i] = acc_x[i] + dx * scale_i;
      acc_y[i] = acc_y[i] + dy * scale_i;
      acc_z[i] = acc_z[i] + dz * scale_i;

      acc_x[j] = acc_x[j] - dx * scale_j;
      acc_y[j] = acc_y[j] - dy * scale_j;
      acc_z[j] = acc_z[j] - dz * scale_j;

      if (dist2.decrypt() < 0.02)
      {
        uint64_t tag = (((uint64_t)step) << 32) | (((uint64_t)i) << 16) | (uint64_t)j;
        bloom_record(tag);
      }
    }
  }

  for (unsigned i = 0; i < BODIES; ++i)
  {
    vel_x[i] = vel_x[i] + DT * acc_x[i];
    vel_y[i] = vel_y[i] + DT * acc_y[i];
    vel_z[i] = vel_z[i] + DT * acc_z[i];

    pos_x[i] = pos_x[i] + DT * vel_x[i];
    pos_y[i] = pos_y[i] + DT * vel_y[i];
    pos_z[i] = pos_z[i] + DT * vel_z[i];
  }
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

  init_system();

  for (unsigned s = 0; s < STEPS; ++s)
    simulate_step(s);

  fp64e_t total_ke = 0;
  fp64e_t momentum_x = 0;
  fp64e_t momentum_y = 0;
  fp64e_t momentum_z = 0;

  for (unsigned i = 0; i < BODIES; ++i)
  {
    fp64e_t v2 = vel_x[i] * vel_x[i] + vel_y[i] * vel_y[i] + vel_z[i] * vel_z[i];
    total_ke = total_ke + 0.5 * mass[i] * v2;

    momentum_x = momentum_x + mass[i] * vel_x[i];
    momentum_y = momentum_y + mass[i] * vel_y[i];
    momentum_z = momentum_z + mass[i] * vel_z[i];
  }

  unsigned bloom_set_bits = 0;
  for (unsigned i = 0; i < BLOOM_BITS; ++i)
    bloom_set_bits += (encounter_bloom[i] != 0).decrypt() ? 1u : 0u;

  libmin_printf("N-body simulation benchmark:\n");
  libmin_printf("  bodies=%u steps=%u dt=%lf\n", BODIES, STEPS, DT);
  libmin_printf("  kinetic_energy=%lf\n", total_ke.decrypt());
  libmin_printf("  momentum=(%lf,%lf,%lf)\n", momentum_x.decrypt(), momentum_y.decrypt(), momentum_z.decrypt());
  libmin_printf("  encrypted_bloom_set_bits=%u/%u\n", bloom_set_bits, BLOOM_BITS);

  libmin_success();
  return 0;
}
