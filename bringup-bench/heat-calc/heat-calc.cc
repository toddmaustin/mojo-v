#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"

#define N 100 // Number of grid points along the rod.
#define STEPS 500 // Number of time steps for the simulation.
#define ALPHA 1.0 // Thermal diffusivity constant.
#define DX 1.0 // Spatial step (distance between grid points).
#define DT 0.1 // Time step (should be small enough for stability).

int main(void)
{
  if (mojov_configure_kmsm_from_dc_fast() != 0)
    return -1;

  if (mojov_enable_and_verify() != 0)
    return -1;

  if (debug_context(SIMON128_KEY, CONTRACT_SIG) != 0)
    return -1;

  fp64e_t u[N]; // Temperature distribution at current time.
  fp64e_t u_new[N]; // Temperature distribution for the next time step.

  int i, step;

  // Initialize the rod:
  // Set an initial temperature distribution with a single "hot spot" at the center.
  // Boundary conditions: fixed at 0.0 at both ends.
  for (i = 0; i < N; i++) {
    if (i == N / 2)
      u[i] = 100.0;
    else
      u[i] = 0.0;
  }

  // Main time-stepping loop: simulate STEPS time steps.
  for (step = 0; step < STEPS; step++) {
    // Update interior points using the explicit finite difference scheme:
    // u_new[i] = u[i] + DT * ALPHA * (u[i-1] - 2*u[i] + u[i+1]) / (DX*DX)
    for (i = 1; i < N - 1; i++) {
      u_new[i] = u[i] + DT * ALPHA * (u[i - 1] - 2.0 * u[i] + u[i + 1]) / (DX * DX);
    }

    // Copy boundary values (Dirichlet boundary conditions; they remain constant).
    u_new[0] = u[0];
    u_new[N - 1] = u[N - 1];

    // Update the current temperature distribution from the newly computed values.
    for (i = 0; i < N; i++) {
      u[i] = u_new[i];
    }
  }

  // Output the final temperature distribution.
  libmin_printf("Final temperature distribution along the rod:\n");
  for (i = 0; i < N; i++) {
    libmin_printf("u[%d] = %.2f\n", i, u[i].decrypt());
  }

  // Compute a simple checksum (sum of all temperatures) for validation.
  fp64e_t checksum = 0.0;
  for (i = 0; i < N; i++) {
    checksum += u[i];
  }

  libmin_printf("Checksum: %.2f\n", checksum.decrypt());

  libmin_success();
  return 0;
}
