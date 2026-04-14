#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"
#include "mojov-math.h"

static int g_failures = 0;
static unsigned g_checks = 0;

static void check_bool(const char *label, int ok)
{
  g_checks++;
  if (!ok)
  {
    g_failures++;
    libmin_printf("MATH-TEST: FAIL[%u] %s\n", g_checks, label);
  }
}

static void check_close(const char *label, fp64e_t got, double expected, double tol)
{
  const double got_d = got.decrypt();
  const double err = libmin_fabs(got_d - expected);
  check_bool(label, err <= tol);
  if (err > tol)
    libmin_printf("MATH-TEST: DETAIL got=%f expected=%f err=%f tol=%f\n", got_d, expected, err, tol);
}

static void check_identity_close(const char *label, fp64e_t a, fp64e_t b, double expected, double tol)
{
  const double got = (a * a + b * b).decrypt();
  const double err = libmin_fabs(got - expected);
  check_bool(label, err <= tol);
  if (err > tol)
    libmin_printf("MATH-TEST: DETAIL got=%f expected=%f err=%f tol=%f\n", got, expected, err, tol);
}

int main(void)
{
  if (mojov_configure_kmsm_from_dc_fast() != 0) return -1;
  if (mojov_enable_and_verify() != 0) return -1;
  if (debug_context(SIMON128_KEY, CONTRACT_SIG) != 0) return -1;

  libmin_printf("MATH-TEST: START mojov-mathtests\n");

  // mojov_fabs
  check_close("mojov_fabs(+3.25)", mojov_fabs(fp64e_t(3.25)), 3.25, 1e-9);
  check_close("mojov_fabs(-3.25)", mojov_fabs(fp64e_t(-3.25)), 3.25, 1e-9);

  // mojov_floor
  check_close("mojov_floor(4.99)", mojov_floor(fp64e_t(4.99)), 4.0, 1e-9);
  check_close("mojov_floor(-4.01)", mojov_floor(fp64e_t(-4.01)), -5.0, 1e-9);
  check_close("mojov_floor(-2.0)", mojov_floor(fp64e_t(-2.0)), -2.0, 1e-9);

  // mojov_pow
  check_close("mojov_pow(2,0)", mojov_pow(fp64e_t(2.0), 0), 1.0, 1e-9);
  check_close("mojov_pow(2,8)", mojov_pow(fp64e_t(2.0), 8), 256.0, 1e-9);
  check_close("mojov_pow(-3,3)", mojov_pow(fp64e_t(-3.0), 3), -27.0, 1e-9);

  // mojov_round (half away from zero)
  check_close("mojov_round(2.49)", mojov_round(fp64e_t(2.49)), 2.0, 1e-9);
  check_close("mojov_round(2.50)", mojov_round(fp64e_t(2.50)), 3.0, 1e-9);
  check_close("mojov_round(-2.49)", mojov_round(fp64e_t(-2.49)), -2.0, 1e-9);
  check_close("mojov_round(-2.50)", mojov_round(fp64e_t(-2.50)), -3.0, 1e-9);

  // mojov_sqrt
  check_close("mojov_sqrt(0)", mojov_sqrt(fp64e_t(0.0)), 0.0, 1e-9);
  check_close("mojov_sqrt(-4)", mojov_sqrt(fp64e_t(-4.0)), 0.0, 1e-9);
  check_close("mojov_sqrt(4)", mojov_sqrt(fp64e_t(4.0)), 2.0, 1e-6);
  check_close("mojov_sqrt(2)", mojov_sqrt(fp64e_t(2.0)), 1.41421356237, 1e-6);
  check_close("mojov_sqrt(0.25)", mojov_sqrt(fp64e_t(0.25)), 0.5, 1e-6);

  // mojov_sin / mojov_cos and _sincos
  const fp64e_t pi = fp64e_t(3.14159265358979323846);
  const fp64e_t half_pi = fp64e_t(1.57079632679489661923);
  const fp64e_t sixth_pi = fp64e_t(0.52359877559829887308);

  check_close("mojov_sin(0)", mojov_sin(fp64e_t(0.0)), 0.0, 2e-4);
  check_close("mojov_sin(pi/2)", mojov_sin(half_pi), 1.0, 2e-3);
  check_close("mojov_sin(-pi/2)", mojov_sin(fp64e_t(-1.57079632679489661923)), -1.0, 2e-3);
  check_close("mojov_sin(pi)", mojov_sin(pi), 0.0, 2e-3);

  check_close("mojov_cos(0)", mojov_cos(fp64e_t(0.0)), 1.0, 2e-3);
  check_close("mojov_cos(pi/2)", mojov_cos(half_pi), 0.0, 2e-3);
  check_close("mojov_cos(pi)", mojov_cos(pi), -1.0, 2e-3);

  check_close("mojov_sin(pi/6)", mojov_sin(sixth_pi), 0.5, 2e-3);
  check_close("mojov_cos(pi/6)", mojov_cos(sixth_pi), 0.86602540378, 2e-3);

  fp64e_t s = fp64e_t(0.0), c = fp64e_t(0.0);
  _sincos(&s, &c, sixth_pi);
  check_close("_sincos sin(pi/6)", s, 0.5, 2e-3);
  check_close("_sincos cos(pi/6)", c, 0.86602540378, 2e-3);
  check_identity_close("sin^2+cos^2 at pi/6", s, c, 1.0, 4e-3);

  if (g_failures != 0)
  {
    libmin_printf("MATH-TEST: FAILURES %d/%u\n", g_failures, g_checks);
    return -1;
  }

  libmin_printf("MATH-TEST: PASS %u checks\n", g_checks);
  libmin_success();
  return 0;
}
