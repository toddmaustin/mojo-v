#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"

#define N_BODIES 3
#define NUM_STEPS 1000
#define DT 0.01
#define G 6.67430e-11

#ifndef EPS
#define EPS 1e-9
#endif

typedef struct {
  fp64e_t mass;
  fp64e_t pos[3];
  fp64e_t vel[3];
} Particle;

int
main(void)
{
  if (mojov_configure_kmsm_from_dc_fast() != 0)
    return -1;

  if (mojov_enable_and_verify() != 0)
    return -1;

  if (debug_context(SIMON128_KEY, CONTRACT_SIG) != 0)
    return -1;

  Particle bodies[N_BODIES] = {
    {1e24, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}},
    {1e24, {1e8, 0.0, 0.0}, {0.0, 1e3, 0.0}},
    {1e24, {0.0, 1e8, 0.0}, {-1e3, 0.0, 0.0}},
  };

  for (int step = 0; step < NUM_STEPS; step++)
  {
    fp64e_t acc[N_BODIES][3] = {{0.0}};

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
