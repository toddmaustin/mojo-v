#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"
#include "mojov-math.h"

#define N_BODIES 3
#define NUM_STEPS 1000
#define DT 0.01
#define G 6.67430e-11

#ifndef EPS
#define EPS 1e-9
#endif

static fp64e_t pos_x[N_BODIES], pos_y[N_BODIES], pos_z[N_BODIES];
static fp64e_t vel_x[N_BODIES], vel_y[N_BODIES], vel_z[N_BODIES];
static fp64e_t mass[N_BODIES];

static void
init_system(void)
{
  for (unsigned i = 0; i < N_BODIES; ++i)
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
}

static void
simulate_step(unsigned step)
{
  fp64e_t acc_x[N_BODIES];
  fp64e_t acc_y[N_BODIES];
  fp64e_t acc_z[N_BODIES];

  for (unsigned i = 0; i < N_BODIES; ++i)
  {
    acc_x[i] = 0;
    acc_y[i] = 0;
    acc_z[i] = 0;
  }

  for (unsigned i = 0; i < N_BODIES; ++i)
  {
    for (unsigned j = i + 1; j < N_BODIES; ++j)
    {
      fp64e_t dx = pos_x[j] - pos_x[i];
      fp64e_t dy = pos_y[j] - pos_y[i];
      fp64e_t dz = pos_z[j] - pos_z[i];

      fp64e_t dist2 = dx * dx + dy * dy + dz * dz + SOFTENING;
      fp64e_t inv_dist = 1.0 / mojov_sqrt(dist2);
      fp64e_t inv_dist3 = inv_dist * inv_dist * inv_dist;
      fp64e_t scale_i = GCONST * mass[j] * inv_dist3;
      fp64e_t scale_j = GCONST * mass[i] * inv_dist3;

      acc_x[i] = acc_x[i] + dx * scale_i;
      acc_y[i] = acc_y[i] + dy * scale_i;
      acc_z[i] = acc_z[i] + dz * scale_i;

      acc_x[j] = acc_x[j] - dx * scale_j;
      acc_y[j] = acc_y[j] - dy * scale_j;
      acc_z[j] = acc_z[j] - dz * scale_j;
    }
  }

  for (unsigned i = 0; i < N_BODIES; ++i)
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

  Particle bodies[N_N_BODIES] = {
    {1e24, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}},
    {1e24, {1e8, 0.0, 0.0}, {0.0, 1e3, 0.0}},
    {1e24, {0.0, 1e8, 0.0}, {-1e3, 0.0, 0.0}},
  };

  for (int step = 0; step < NUM_STEPS; step++)
  {
    fp64e_t acc[N_N_BODIES][3] = {{0.0}};

    for (int i = 0; i < N_BODIES; i++)
    {
      for (int j = 0; j < N_BODIES; j++)
      {
        if (i == j)
          continue;

        fp64e_t dx = bodies[j].pos[0] - bodies[i].pos[0];
        fp64e_t dy = bodies[j].pos[1] - bodies[i].pos[1];
        fp64e_t dz = bodies[j].pos[2] - bodies[i].pos[2];

        fp64e_t r2 = dx * dx + dy * dy + dz * dz + EPS;
        fp64e_t r = libmin_sqrt(r2);

        fp64e_t a = G * bodies[j].mass / r2;
        acc[i][0] = acc[i][0] + a * (dx / r);
        acc[i][1] = acc[i][1] + a * (dy / r);
        acc[i][2] = acc[i][2] + a * (dz / r);
      }
    }

    for (int i = 0; i < N_BODIES; i++)
    {
      bodies[i].vel[0] = bodies[i].vel[0] + acc[i][0] * DT;
      bodies[i].vel[1] = bodies[i].vel[1] + acc[i][1] * DT;
      bodies[i].vel[2] = bodies[i].vel[2] + acc[i][2] * DT;
      bodies[i].pos[0] = bodies[i].pos[0] + bodies[i].vel[0] * DT;
      bodies[i].pos[1] = bodies[i].pos[1] + bodies[i].vel[1] * DT;
      bodies[i].pos[2] = bodies[i].pos[2] + bodies[i].vel[2] * DT;
    }
  }

  libmin_printf("Final state after %d steps:\n", NUM_STEPS);
  for (int i = 0; i < N_BODIES; i++)
  {
    libmin_printf("Body %d:\n", i);
    libmin_printf(" Position = (%f, %f, %f) m\n", bodies[i].pos[0].decrypt(), bodies[i].pos[1].decrypt(),
                  bodies[i].pos[2].decrypt());
    libmin_printf(" Velocity = (%f, %f, %f) m/s\n\n", bodies[i].vel[0].decrypt(), bodies[i].vel[1].decrypt(),
                  bodies[i].vel[2].decrypt());
  }

  libmin_success();
  return 0;
}
