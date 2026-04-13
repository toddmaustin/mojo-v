#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"
#include "mojov-math.h"

typedef int8e_t VIP_ENCCHAR;
typedef uint64e_t VIP_ENCBOOL;
typedef uint64e_t VIP_ENCUINT64;
typedef fp32e_t VIP_ENCFLOAT;
typedef fp64e_t VIP_ENCDOUBLE;

#define VERSION "1.11"

#ifndef PI
#define PI 3.14159265359
#endif

#define LAPLACE_LIMIT .6627434193
#define MAXITER 16

static double derror = 0.000001;

static double bin_fact(int n, int k);
static VIP_ENCDOUBLE J(int n, VIP_ENCDOUBLE x);

static VIP_ENCDOUBLE strict_iteration(VIP_ENCDOUBLE E, VIP_ENCDOUBLE e, VIP_ENCDOUBLE M, int reset)
{
  (void)E;
  (void)reset;
  return M + e * mojov_sin(E);
}

static VIP_ENCDOUBLE newton(VIP_ENCDOUBLE E, VIP_ENCDOUBLE e, VIP_ENCDOUBLE M, int reset)
{
  (void)reset;
  return E + (M + e * mojov_sin(E) - E) / ((VIP_ENCDOUBLE)1 - e * mojov_cos(E));
}

static VIP_ENCDOUBLE binary(VIP_ENCDOUBLE E, VIP_ENCDOUBLE e, VIP_ENCDOUBLE M, int reset)
{
  static double scale = .7853981633975;
  VIP_ENCDOUBLE R;

  if (reset) {
    scale = PI / 4.0;
    return (VIP_ENCDOUBLE)0.0;
  }

  R = E - e * mojov_sin(E);
  E = cmov(M > R, (E + scale), (E - scale));
  scale = scale / 2.0;
  return E;
}

static VIP_ENCDOUBLE e_series(VIP_ENCDOUBLE E, VIP_ENCDOUBLE e, VIP_ENCDOUBLE M, int reset)
{
  static int n;
  int k;
  VIP_ENCDOUBLE n_2k;
  VIP_ENCDOUBLE a_n = 0.0;
  VIP_ENCDOUBLE s_k;

  if (reset) {
    n = 0;
    return (VIP_ENCDOUBLE)0.0;
  }

  if (n == 0) {
    n++;
    return M;
  }

  for (k = 0; 2 * k <= n; k++) {
    n_2k = (double)n - 2.0 * (double)k;
    s_k = (k % 2) ? -1.0 : 1.0;
    a_n = a_n + (s_k * bin_fact(n, k) * mojov_sin(n_2k * M));
  }

  n++;
  return E + mypow(e, n - 1U) * a_n;
}

static VIP_ENCDOUBLE j_series(VIP_ENCDOUBLE E, VIP_ENCDOUBLE e, VIP_ENCDOUBLE M, int reset)
{
  static int n;
  VIP_ENCDOUBLE dn;
  VIP_ENCDOUBLE term;

  if (reset) {
    n = 0;
    return (VIP_ENCDOUBLE)0.0;
  }

  if (n == 0) {
    n++;
    return M;
  }

  dn = (double)n;
  term = ((VIP_ENCDOUBLE)2.0 / (double)n) * J(n, dn * e) * mojov_sin(dn * M);
  n++;
  return E + term;
}

typedef VIP_ENCDOUBLE (*method_fn)(VIP_ENCDOUBLE, VIP_ENCDOUBLE, VIP_ENCDOUBLE, int);

static method_fn methods[] = {
  strict_iteration,
  newton,
  binary,
  e_series,
  j_series,
};

#define NMETHODS (sizeof(methods) / sizeof(methods[0]))

static int newmain(int argc, const char **argv)
{
  int i = 1;
  int m = 1;
  VIP_ENCDOUBLE sign = 1.0;
  VIP_ENCDOUBLE M = 0.0;
  double _e = -0.1;
  VIP_ENCDOUBLE e = -0.1;
  VIP_ENCDOUBLE E_old = PI / 2;
  VIP_ENCDOUBLE E = 0.0;
  method_fn method;

  while (argv[i][0] == '-') {
    if (strcmp(argv[i], "-a") == 0) {
      derror = atof(argv[i + 1]);
      i += 2;
      continue;
    }

    if (strcmp(argv[i], "-m") == 0) {
      m = atoi(argv[i + 1]);
      if ((m <= 0) || (m > (int)NMETHODS)) {
        libmin_printf("Bad method number %d\n", m);
        return 1;
      }
      i += 2;
      continue;
    }

    libmin_printf("kepler: Unknown option %s\n", argv[i]);
    return 1;
  }

  if (i + 2 > argc) {
    libmin_printf("Usage: kepler -m <1..5> M e\n");
    return 1;
  }

  M = atof(argv[i++]);
  e = _e = atof(argv[i]);
  method = methods[m - 1];

  if ((m == 4) && (_e > LAPLACE_LIMIT)) {
    libmin_printf("e cannot exceed %f for this method.\n", LAPLACE_LIMIT);
    return 1;
  }

  if ((_e < 0) || (_e >= 1.0)) {
    libmin_printf("Eccentricity %f out of range.\n", _e);
    return 1;
  }

  sign = cmov(M > 0, (VIP_ENCDOUBLE)1.0, (VIP_ENCDOUBLE)-1.0);
  M = myfabs(M) / ((VIP_ENCDOUBLE)2 * PI);
  M = (M - myfloor(M)) * 2 * PI * sign;

  sign = 1.0;
  {
    VIP_ENCBOOL pred = M > PI;
    M = cmov(pred, (VIP_ENCDOUBLE)2 * PI - M, M);
    sign = cmov(pred, (VIP_ENCDOUBLE)-1.0, sign);
  }

  for (unsigned iter = 0; iter < MAXITER; iter++) {
    E = method(E_old, e, M, 0);
    E_old = E;
  }

  libmin_printf("E = %f\n", (sign * E).decrypt());
  (void)derror;
  return 0;
}

int main(void)
{
  if (mojov_configure_kmsm_from_dc_fast() != 0)
    return -1;

  if (mojov_enable_and_verify() != 0)
    return -1;

  if (debug_context(SIMON128_KEY, CONTRACT_SIG) != 0)
    return -1;

  libmin_srand(42);

  libmin_printf("Solve Kepler's Eq via simple iteration for test parameters...\n");
  {
    int argc = 5;
    const char *argv[] = {"kepler", "-m", "1", "0.34", "0.25"};
    newmain(argc, argv);
  }

  libmin_printf("Solve Kepler's Eq via Newton's method for SpaceX Tesla...\n");
  {
    int argc = 5;
    const char *argv[] = {"kepler", "-m", "2", "6.037831992006549", "0.25600674983752"};
    newmain(argc, argv);
  }

  libmin_printf("Solve Kepler's Eq via binary search for Haley's comet...\n");
  {
    int argc = 5;
    const char *argv[] = {"kepler", "-m", "3", "0.66985737", "0.96714"};
    newmain(argc, argv);
  }

  libmin_printf("Solve Kepler's Eq via power series for Earth's orbit...\n");
  {
    int argc = 5;
    const char *argv[] = {"kepler", "-m", "4", "6.259047404", "0.0167086"};
    newmain(argc, argv);
  }

  libmin_printf("Solve Kepler's Eq via Fourier Bessel series for Pluto's orbit...\n");
  {
    int argc = 5;
    const char *argv[] = {"kepler", "-m", "5", "0.25359634", "0.2488"};
    newmain(argc, argv);
  }

  libmin_success();
  return 0;
}

static double bin_fact(int n, int k)
{
  int j;
  double cum_prod = 1.0;
  double num_fact;
  double den_fact;
  double dj;
  double dk;
  double x;

  x = ((double)n) / 2.0 - (double)k;
  for (j = n - k; j > 1; j--) {
    dj = (double)j;
    dk = (double)n - (double)k - dj + 1.0;
    den_fact = n - k - j + 1 <= k ? dk * dj : dj;
    num_fact = n - k - j + 1 <= k ? x * x : x;
    cum_prod = cum_prod * (num_fact / den_fact);
  }

  return cum_prod;
}

#define MAXJITER 12

static VIP_ENCDOUBLE J(int n, VIP_ENCDOUBLE x)
{
  VIP_ENCDOUBLE dsum = 0.0;
  VIP_ENCDOUBLE dterm;
  VIP_ENCDOUBLE s_j;
  VIP_ENCDOUBLE d_n;
  VIP_ENCDOUBLE d_j;
  VIP_ENCDOUBLE cfact = 1.0;
  int j;
  int nn;

  nn = n >= 0 ? n : -n;
  d_n = (double)nn;

  for (j = 1; j <= nn; j++) {
    d_j = (double)j;
    cfact = cfact * (x / ((VIP_ENCDOUBLE)2.0 * d_j));
  }

  dsum = dterm = cfact;

  for (j = 1; j < MAXJITER; j++) {
    d_j = (double)j;
    s_j = (j % 2) ? -1.0 : 1.0;
    dterm = dterm * (x * x / (d_j * 4.0 * (d_n + d_j)));
    dsum = dsum + (s_j * dterm);
  }

  s_j = (nn % 2) ? -1.0 : 1.0;
  return n >= 0 ? dsum : s_j * dsum;
}
