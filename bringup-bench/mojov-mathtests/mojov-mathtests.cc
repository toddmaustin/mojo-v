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

static void check_u64(const char *label, uint64e_t got, uint64_t expected)
{
  const uint64_t got_u = got.decrypt();
  check_bool(label, got_u == expected);
  if (got_u != expected)
    libmin_printf("MATH-TEST: DETAIL got=%llu expected=%llu\n",
                  (unsigned long long) got_u,
                  (unsigned long long) expected);
}

static void check_i64(const char *label, int64e_t got, int64_t expected)
{
  const int64_t got_i = got.decrypt();
  check_bool(label, got_i == expected);
  if (got_i != expected)
    libmin_printf("MATH-TEST: DETAIL got=%lld expected=%lld\n",
                  (long long) got_i,
                  (long long) expected);
}

int main(void)
{
  if (mojov_configure_kmsm_from_dc_fast() != 0) return -1;
  if (mojov_enable_and_verify() != 0) return -1;
  if (debug_context(SIMON128_KEY, CONTRACT_SIG) != 0) return -1;

  libmin_printf("MATH-TEST: START mojov-mathtests\n");

  // 1) Core scalar helpers
  check_close("mojov_abs(-3.25)", mojov_abs(fp64e_t(-3.25)), 3.25, 1e-9);
  check_close("mojov_abs(0)", mojov_abs(fp64e_t(0.0)), 0.0, 1e-9);
  check_close("mojov_abs(4.5)", mojov_abs(fp64e_t(4.5)), 4.5, 1e-9);
  check_close("mojov_min(1.5,-2.0)", mojov_min(fp64e_t(1.5), fp64e_t(-2.0)), -2.0, 1e-9);
  check_close("mojov_min(eq)", mojov_min(fp64e_t(2.0), fp64e_t(2.0)), 2.0, 1e-9);
  check_close("mojov_max(1.5,-2.0)", mojov_max(fp64e_t(1.5), fp64e_t(-2.0)), 1.5, 1e-9);
  check_close("mojov_max(eq)", mojov_max(fp64e_t(-2.0), fp64e_t(-2.0)), -2.0, 1e-9);
  check_close("mojov_clamp(mid)", mojov_clamp(fp64e_t(0.5), fp64e_t(-1.0), fp64e_t(2.0)), 0.5, 1e-9);
  check_close("mojov_clamp(low)", mojov_clamp(fp64e_t(-5.0), fp64e_t(-1.0), fp64e_t(2.0)), -1.0, 1e-9);
  check_close("mojov_clamp(high)", mojov_clamp(fp64e_t(5.0), fp64e_t(-1.0), fp64e_t(2.0)), 2.0, 1e-9);
  check_close("mojov_copysign", mojov_copysign(fp64e_t(2.5), fp64e_t(-1.0)), -2.5, 1e-9);
  check_close("mojov_copysign(-0)", mojov_copysign(fp64e_t(2.5), fp64e_t(-0.0)), -2.5, 1e-9);
  check_u64("mojov_signbit(+0.0)", mojov_signbit(fp64e_t(0.0)), 0u);
  check_u64("mojov_signbit(-0.0)", mojov_signbit(fp64e_t(-0.0)), 1u);
  check_u64("mojov_signbit(-8)", mojov_signbit(fp64e_t(-8.0)), 1u);

  // 2) Rounding and conversion
  check_close("mojov_floor(-4.01)", mojov_floor(fp64e_t(-4.01)), -5.0, 1e-9);
  check_close("mojov_floor(3)", mojov_floor(fp64e_t(3.0)), 3.0, 1e-9);
  check_close("mojov_ceil(-4.01)", mojov_ceil(fp64e_t(-4.01)), -4.0, 1e-9);
  check_close("mojov_ceil(3)", mojov_ceil(fp64e_t(3.0)), 3.0, 1e-9);
  check_close("mojov_trunc(-4.99)", mojov_trunc(fp64e_t(-4.99)), -4.0, 1e-9);
  check_close("mojov_trunc(4.99)", mojov_trunc(fp64e_t(4.99)), 4.0, 1e-9);
  check_close("mojov_round(2.50)", mojov_round(fp64e_t(2.50)), 3.0, 1e-9);
  check_close("mojov_round(2.49)", mojov_round(fp64e_t(2.49)), 2.0, 1e-9);
  check_close("mojov_round(-2.49)", mojov_round(fp64e_t(-2.49)), -2.0, 1e-9);
  check_close("mojov_round(-2.50)", mojov_round(fp64e_t(-2.50)), -3.0, 1e-9);
  check_i64("mojov_lrint(3.6)", mojov_lrint(fp64e_t(3.6)), 4);
  check_i64("mojov_lrint(-3.6)", mojov_lrint(fp64e_t(-3.6)), -4);
  check_close("mojov_from_int(-7)", mojov_from_int(int64e_t(-7)), -7.0, 1e-9);
  check_close("mojov_from_int(0)", mojov_from_int(int64e_t(0)), 0.0, 1e-9);

  // 3) Elementary numeric kernels
  check_close("mojov_sqrt(4)", mojov_sqrt(fp64e_t(4.0)), 2.0, 10e-5);
  check_close("mojov_sqrt(0)", mojov_sqrt(fp64e_t(0.0)), 0.0, 1e-9);
  check_close("mojov_sqrt(-1)", mojov_sqrt(fp64e_t(-1.0)), 0.0, 1e-9);
  check_close("mojov_rsqrt(4)", mojov_rsqrt(fp64e_t(4.0)), 0.5, 10e-5);
  check_close("mojov_rsqrt(0)", mojov_rsqrt(fp64e_t(0.0)), 0.0, 1e-9);
  check_close("mojov_recip(4)", mojov_recip(fp64e_t(4.0)), 0.25, 1e-9);
  check_close("mojov_recip(-2)", mojov_recip(fp64e_t(-2.0)), -0.5, 1e-9);
  check_close("mojov_fma", mojov_fma(fp64e_t(2.0), fp64e_t(3.0), fp64e_t(4.0)), 10.0, 1e-9);
  check_close("mojov_hypot(3,4)", mojov_hypot(fp64e_t(3.0), fp64e_t(4.0)), 5.0, 10e-5);
  check_close("mojov_hypot(0,0)", mojov_hypot(fp64e_t(0.0), fp64e_t(0.0)), 0.0, 1e-9);

  // 4) Exponential/logarithmic family
  check_close("mojov_exp(1)", mojov_exp(fp64e_t(1.0)), 2.71828182845904523536, 1e-9);
  check_close("mojov_exp(0)", mojov_exp(fp64e_t(0.0)), 1.0, 1e-9);
  check_close("mojov_exp(-1)", mojov_exp(fp64e_t(-1.0)), 0.36787944117144232159, 1e-8);
  check_close("mojov_exp2(5)", mojov_exp2(fp64e_t(5.0)), 32.0, 1e-9);
  check_close("mojov_exp2(-3)", mojov_exp2(fp64e_t(-3.0)), 0.125, 1e-8);
  check_close("mojov_log(e)", mojov_log(fp64e_t(2.71828182845904523536)), 1.0, 1e-9);
  check_close("mojov_log(1)", mojov_log(fp64e_t(1.0)), 0.0, 1e-9);
  check_close("mojov_log(0)", mojov_log(fp64e_t(0.0)), 0.0, 1e-9);
  check_close("mojov_log(-2)", mojov_log(fp64e_t(-2.0)), 0.0, 1e-9);
  check_close("mojov_log2(8)", mojov_log2(fp64e_t(8.0)), 3.0, 1e-9);
  check_close("mojov_log2(1)", mojov_log2(fp64e_t(1.0)), 0.0, 1e-9);
  check_close("mojov_log10(1000)", mojov_log10(fp64e_t(1000.0)), 3.0, 1e-9);
  check_close("mojov_log10(1)", mojov_log10(fp64e_t(1.0)), 0.0, 1e-9);
  check_close("mojov_expm1(0.5)", mojov_expm1(fp64e_t(0.5)), 0.64872127070012819416, 1e-9);
  check_close("mojov_expm1(0)", mojov_expm1(fp64e_t(0.0)), 0.0, 1e-9);
  check_close("mojov_log1p(0.5)", mojov_log1p(fp64e_t(0.5)), 0.40546510810816438198, 1e-9);
  check_close("mojov_log1p(0)", mojov_log1p(fp64e_t(0.0)), 0.0, 1e-9);

  // 5) Trigonometric family
  const fp64e_t pi = fp64e_t(3.14159265358979323846);
  const fp64e_t half_pi = fp64e_t(1.57079632679489661923);
  const fp64e_t sixth_pi = fp64e_t(0.52359877559829887308);

  check_close("mojov_sin(0)", mojov_sin(fp64e_t(0.0)), 0.0, 2e-4);
  check_close("mojov_sin(-pi/2)", mojov_sin(-half_pi), -1.0, 2e-3);
  check_close("mojov_sin(pi/2)", mojov_sin(half_pi), 1.0, 2e-3);
  check_close("mojov_cos(0)", mojov_cos(fp64e_t(0.0)), 1.0, 2e-3);
  check_close("mojov_cos(pi)", mojov_cos(pi), -1.0, 2e-3);
  check_close("mojov_sin(2pi)", mojov_sin(fp64e_t(6.28318530717958647692)), 0.0, 4e-3);
  check_close("mojov_cos(2pi)", mojov_cos(fp64e_t(6.28318530717958647692)), 1.0, 4e-3);

  fp64e_t s = fp64e_t(0.0), c = fp64e_t(0.0);
  _sincos(&s, &c, sixth_pi);
  check_close("_sincos sin(pi/6)", s, 0.5, 2e-3);
  check_close("_sincos cos(pi/6)", c, 0.86602540378, 2e-3);

  // 6) Power interfaces
  check_close("mojov_powi(2,8)", mojov_powi(fp64e_t(2.0), 8), 256.0, 1e-9);
  check_close("mojov_powi(5,0)", mojov_powi(fp64e_t(5.0), 0), 1.0, 1e-9);
  check_close("mojov_pow(3,4)", mojov_pow(fp64e_t(3.0), 4), 81.0, 1e-9);
  check_close("mojov_square(3)", mojov_square(fp64e_t(3.0)), 9.0, 1e-9);
  check_close("mojov_square(-3)", mojov_square(fp64e_t(-3.0)), 9.0, 1e-9);
  check_close("mojov_cube(-2)", mojov_cube(fp64e_t(-2.0)), -8.0, 1e-9);
  check_close("mojov_cube(0)", mojov_cube(fp64e_t(0.0)), 0.0, 1e-9);

  // 7) Classification / sanitization
  const fp64e_t inf = fp64e_t(1.0) / fp64e_t(0.0);
  const fp64e_t nan = fp64e_t(0.0) / fp64e_t(0.0);
  check_u64("mojov_iszero(0)", mojov_iszero(fp64e_t(0.0)), 1u);
  check_u64("mojov_iszero(-0)", mojov_iszero(fp64e_t(-0.0)), 1u);
  check_u64("mojov_iszero(2)", mojov_iszero(fp64e_t(2.0)), 0u);
  check_u64("mojov_isfinite(1)", mojov_isfinite(fp64e_t(1.0)), 1u);
  check_u64("mojov_isfinite(nan)", mojov_isfinite(nan), 0u);
  check_u64("mojov_isfinite(inf)", mojov_isfinite(inf), 0u);
  check_u64("mojov_isnan(nan)", mojov_isnan(nan), 1u);
  check_u64("mojov_isnan(1)", mojov_isnan(fp64e_t(1.0)), 0u);
  check_u64("mojov_isinf(inf)", mojov_isinf(inf), 1u);
  check_u64("mojov_isinf(1)", mojov_isinf(fp64e_t(1.0)), 0u);
  check_close("mojov_safe_div normal", mojov_safe_div(fp64e_t(10.0), fp64e_t(2.0), fp64e_t(7.0)), 5.0, 1e-9);
  check_close("mojov_safe_div fallback", mojov_safe_div(fp64e_t(1.0), fp64e_t(0.0), fp64e_t(7.0)), 7.0, 1e-9);
  check_close("mojov_safe_log normal", mojov_safe_log(fp64e_t(1.0), fp64e_t(3.0)), 0.0, 1e-9);
  check_close("mojov_safe_log fallback", mojov_safe_log(fp64e_t(-1.0), fp64e_t(3.0)), 3.0, 1e-9);
  check_close("mojov_safe_sqrt normal", mojov_safe_sqrt(fp64e_t(9.0), fp64e_t(5.0)), 3.0, 10e-5);
  check_close("mojov_safe_sqrt fallback", mojov_safe_sqrt(fp64e_t(-1.0), fp64e_t(5.0)), 5.0, 1e-9);

  // 8) ML / privacy-analytics helpers
  check_close("mojov_sigmoid(0)", mojov_sigmoid(fp64e_t(0.0)), 0.5, 1e-9);
  check_close("mojov_sigmoid(8)", mojov_sigmoid(fp64e_t(8.0)), 0.9996646498695336, 5e-6);
  check_close("mojov_sigmoid(-8)", mojov_sigmoid(fp64e_t(-8.0)), 0.0003353501304664781, 5e-6);
  check_close("mojov_tanh(0.5)", mojov_tanh(fp64e_t(0.5)), 0.46211715726000975850, 1e-9);
  check_close("mojov_tanh(-0.5)", mojov_tanh(fp64e_t(-0.5)), -0.46211715726000975850, 1e-9);
  check_close("mojov_relu(2)", mojov_relu(fp64e_t(2.0)), 2.0, 1e-9);
  check_close("mojov_relu(-2)", mojov_relu(fp64e_t(-2.0)), 0.0, 1e-9);
  check_close("mojov_leaky_relu(2,0.1)", mojov_leaky_relu(fp64e_t(2.0), fp64e_t(0.1)), 2.0, 1e-9);
  check_close("mojov_leaky_relu(-2,0.1)", mojov_leaky_relu(fp64e_t(-2.0), fp64e_t(0.1)), -0.2, 1e-9);
  check_close("mojov_softplus(0)", mojov_softplus(fp64e_t(0.0)), 0.69314718055994530942, 1e-9);
  check_close("mojov_softplus(-2)", mojov_softplus(fp64e_t(-2.0)), 0.1269280110429725, 1e-8);

  if (g_failures != 0)
  {
    libmin_printf("MATH-TEST: FAILURES %d/%u\n", g_failures, g_checks);
    return -1;
  }

  libmin_printf("MATH-TEST: PASS %u checks\n", g_checks);
  libmin_success();
  return 0;
}
