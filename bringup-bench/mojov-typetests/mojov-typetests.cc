#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"

using namespace exo;

static int g_failures = 0;

static void check_bool(const char *label, bool ok)
{
  libmin_printf("TEST: %s : %s\n", label, ok ? "PASS" : "FAIL");
  if (!ok) g_failures++;
}

#define CHECK_INT(LABEL, EXPR, EXPECTED) \
  do { \
    auto _v = (EXPR); \
    auto _d = _v.decrypt(); \
    auto _e = (EXPECTED); \
    check_bool((LABEL), _d == (decltype(_d))(_e)); \
  } while (0)

#define CHECK_FP(LABEL, EXPR, EXPECTED) \
  do { \
    auto _v = (EXPR); \
    auto _d = _v.decrypt(); \
    auto _e = (EXPECTED); \
    check_bool((LABEL), _d == (decltype(_d))(_e)); \
  } while (0)

template <typename T>
static void run_unsigned_ops(const char *name, typename T::value_type a_in, typename T::value_type b_in)
{
  T a = a_in, b = b_in;
  auto pred = inte_t<64, false>(a < b);
  CHECK_INT(name, +a, a_in);
  CHECK_INT(name, ~a, (typename T::value_type)(~a_in));
  CHECK_INT(name, !a, (typename T::value_type)(!a_in));

  CHECK_INT(name, a + b, (typename T::value_type)(a_in + b_in));
  CHECK_INT(name, a - b, (typename T::value_type)(a_in - b_in));
  CHECK_INT(name, a * b, (typename T::value_type)(a_in * b_in));
  CHECK_INT(name, a / b, (typename T::value_type)(a_in / b_in));
  CHECK_INT(name, a % b, (typename T::value_type)(a_in % b_in));
  CHECK_INT(name, a & b, (typename T::value_type)(a_in & b_in));
  CHECK_INT(name, a | b, (typename T::value_type)(a_in | b_in));
  CHECK_INT(name, a ^ b, (typename T::value_type)(a_in ^ b_in));
  CHECK_INT(name, a << 1, (typename T::value_type)(a_in << 1));
  CHECK_INT(name, a >> 1, (typename T::value_type)(a_in >> 1));

  CHECK_INT(name, a + b_in, (typename T::value_type)(a_in + b_in));
  CHECK_INT(name, a - b_in, (typename T::value_type)(a_in - b_in));
  CHECK_INT(name, a * b_in, (typename T::value_type)(a_in * b_in));
  CHECK_INT(name, a / b_in, (typename T::value_type)(a_in / b_in));
  CHECK_INT(name, a % b_in, (typename T::value_type)(a_in % b_in));
  CHECK_INT(name, a_in + b, (typename T::value_type)(a_in + b_in));
  CHECK_INT(name, a_in - b, (typename T::value_type)(a_in - b_in));
  CHECK_INT(name, a_in * b, (typename T::value_type)(a_in * b_in));
  CHECK_INT(name, a_in / b, (typename T::value_type)(a_in / b_in));
  CHECK_INT(name, a_in % b, (typename T::value_type)(a_in % b_in));

  CHECK_INT(name, a == b, (uint64_t)(a_in == b_in));
  CHECK_INT(name, a != b, (uint64_t)(a_in != b_in));
  CHECK_INT(name, a < b, (uint64_t)(a_in < b_in));
  CHECK_INT(name, a <= b, (uint64_t)(a_in <= b_in));
  CHECK_INT(name, a > b, (uint64_t)(a_in > b_in));
  CHECK_INT(name, a >= b, (uint64_t)(a_in >= b_in));

  CHECK_INT(name, a < (typename T::value_type)(a_in + b_in), 1u);
  CHECK_INT(name, a > (typename T::value_type)(a_in + b_in), 0u);

  CHECK_INT(name, a && b, (typename T::value_type)((a_in && b_in) ? 1 : 0));
  CHECK_INT(name, a || b, (typename T::value_type)((a_in || b_in) ? 1 : 0));

  T x = a;
  x = b;
  CHECK_INT(name, x, b_in);
  x = a;
  x += b;
  CHECK_INT(name, x, (typename T::value_type)(a_in + b_in));
  x = a;
  x -= b;
  CHECK_INT(name, x, (typename T::value_type)(a_in - b_in));
  x = a;
  x *= b;
  CHECK_INT(name, x, (typename T::value_type)(a_in * b_in));
  x = a;
  x /= b;
  CHECK_INT(name, x, (typename T::value_type)(a_in / b_in));
  x = a;
  x %= b;
  CHECK_INT(name, x, (typename T::value_type)(a_in % b_in));
  x = a;
  x &= b;
  CHECK_INT(name, x, (typename T::value_type)(a_in & b_in));
  x = a;
  x |= b;
  CHECK_INT(name, x, (typename T::value_type)(a_in | b_in));
  x = a;
  x ^= b;
  CHECK_INT(name, x, (typename T::value_type)(a_in ^ b_in));
  x = a;
  x <<= 1;
  CHECK_INT(name, x, (typename T::value_type)(a_in << 1));
  x = a;
  x >>= 1;
  CHECK_INT(name, x, (typename T::value_type)(a_in >> 1));

  x = a;
  ++x;
  CHECK_INT(name, x, (typename T::value_type)(a_in + 1));
  x = a;
  x++;
  CHECK_INT(name, x, (typename T::value_type)(a_in + 1));
  x = a;
  --x;
  CHECK_INT(name, x, (typename T::value_type)(a_in - 1));
  x = a;
  x--;
  CHECK_INT(name, x, (typename T::value_type)(a_in - 1));

  CHECK_INT(name, cmov(pred, a, b), (typename T::value_type)(a_in < b_in ? a_in : b_in));
  CHECK_INT(name, cmov(pred, (typename T::value_type)(a_in), b), (typename T::value_type)(a_in < b_in ? a_in : b_in));
  CHECK_INT(name, cmov(pred, a, (typename T::value_type)(b_in)), (typename T::value_type)(a_in < b_in ? a_in : b_in));
  CHECK_INT(name, cmov(true, a, b), a_in);
}

template <typename T>
static void run_signed_ops(const char *name, typename T::value_type a_in, typename T::value_type b_in)
{
  T a = a_in, b = b_in;
  auto pred = inte_t<64, false>(a < b);
  CHECK_INT(name, +a, a_in);
  CHECK_INT(name, -a, (typename T::value_type)(-a_in));
  CHECK_INT(name, ~a, (typename T::value_type)(~a_in));
  CHECK_INT(name, !a, (typename T::value_type)(!a_in));

  CHECK_INT(name, a + b, (typename T::value_type)(a_in + b_in));
  CHECK_INT(name, a - b, (typename T::value_type)(a_in - b_in));
  CHECK_INT(name, a * b, (typename T::value_type)(a_in * b_in));
  CHECK_INT(name, a / b, (typename T::value_type)(a_in / b_in));
  CHECK_INT(name, a % b, (typename T::value_type)(a_in % b_in));
  CHECK_INT(name, a >> 1, (typename T::value_type)(a_in >> 1));

  CHECK_INT(name, a < b, (uint64_t)(a_in < b_in));
  CHECK_INT(name, a <= b, (uint64_t)(a_in <= b_in));
  CHECK_INT(name, a > b, (uint64_t)(a_in > b_in));
  CHECK_INT(name, a >= b, (uint64_t)(a_in >= b_in));
  CHECK_INT(name, a == b, (uint64_t)(a_in == b_in));
  CHECK_INT(name, a != b, (uint64_t)(a_in != b_in));

  CHECK_INT(name, a < (typename T::value_type)(-1), (uint64_t)(a_in < -1));
  CHECK_INT(name, a > (typename T::value_type)(-1), (uint64_t)(a_in > -1));

  T x = a;
  x = b;
  CHECK_INT(name, x, b_in);
  x = a; x += b; CHECK_INT(name, x, (typename T::value_type)(a_in + b_in));
  x = a; x -= b; CHECK_INT(name, x, (typename T::value_type)(a_in - b_in));
  x = a; x *= b; CHECK_INT(name, x, (typename T::value_type)(a_in * b_in));
  x = a; x /= b; CHECK_INT(name, x, (typename T::value_type)(a_in / b_in));
  x = a; x %= b; CHECK_INT(name, x, (typename T::value_type)(a_in % b_in));
  x = a; x >>= 1; CHECK_INT(name, x, (typename T::value_type)(a_in >> 1));

  CHECK_INT(name, cmov(pred, a, b), (typename T::value_type)(a_in < b_in ? a_in : b_in));
}

template <typename T>
static void run_fp_ops(const char *name, typename T::value_type a_in, typename T::value_type b_in)
{
  T a = a_in, b = b_in;
  auto pred = inte_t<64, false>(a < b);
  CHECK_FP(name, +a, a_in);
  CHECK_FP(name, -a, (typename T::value_type)(-a_in));
  CHECK_FP(name, a + b, (typename T::value_type)(a_in + b_in));
  CHECK_FP(name, a - b, (typename T::value_type)(a_in - b_in));
  CHECK_FP(name, a * b, (typename T::value_type)(a_in * b_in));
  CHECK_FP(name, a / b, (typename T::value_type)(a_in / b_in));

  CHECK_FP(name, a + b_in, (typename T::value_type)(a_in + b_in));
  CHECK_FP(name, a_in + b, (typename T::value_type)(a_in + b_in));
  CHECK_FP(name, a - b_in, (typename T::value_type)(a_in - b_in));
  CHECK_FP(name, a_in - b, (typename T::value_type)(a_in - b_in));

  CHECK_INT(name, a < b, (uint64_t)(a_in < b_in));
  CHECK_INT(name, a <= b, (uint64_t)(a_in <= b_in));
  CHECK_INT(name, a == b, (uint64_t)(a_in == b_in));
  CHECK_INT(name, a > b, (uint64_t)(a_in > b_in));
  CHECK_INT(name, a >= b, (uint64_t)(a_in >= b_in));
  CHECK_INT(name, a != b, (uint64_t)(a_in != b_in));

  T x = a;
  x = b;
  CHECK_FP(name, x, b_in);
  x = a; x += b; CHECK_FP(name, x, (typename T::value_type)(a_in + b_in));
  x = a; x -= b; CHECK_FP(name, x, (typename T::value_type)(a_in - b_in));
  x = a; x *= b; CHECK_FP(name, x, (typename T::value_type)(a_in * b_in));
  x = a; x /= b; CHECK_FP(name, x, (typename T::value_type)(a_in / b_in));

  CHECK_FP(name, cmov(pred, a, b), (typename T::value_type)(a_in < b_in ? a_in : b_in));
  CHECK_FP(name, cmov(pred, (typename T::value_type)(a_in), b), (typename T::value_type)(a_in < b_in ? a_in : b_in));
  CHECK_FP(name, cmov(pred, a, (typename T::value_type)(b_in)), (typename T::value_type)(a_in < b_in ? a_in : b_in));
}

#define TEST_CAST(SRC, DST, V, LABEL) do { SRC s = (V); DST d = s; check_bool(LABEL, d.decrypt() == (typename DST::value_type)(V)); } while (0)

int main(void)
{
  if (mojov_configure_kmsm_from_dc_fast() != 0) return -1;
  if (mojov_enable_and_verify() != 0) return -1;
  if (debug_context(SIMON128_KEY, CONTRACT_SIG) != 0) return -1;

  libmin_printf("INFO: Running encrypted type tests.\n");

  run_unsigned_ops<uint8e_t>("u8 operators", 21, 5);
  run_unsigned_ops<uint16e_t>("u16 operators", 300, 7);
  run_unsigned_ops<uint32e_t>("u32 operators", 70000, 97);
  run_unsigned_ops<uint64e_t>("u64 operators", 1000000000ull, 257ull);

  run_signed_ops<int8e_t>("i8 operators", -61, 5);
  run_signed_ops<int16e_t>("i16 operators", -5000, 11);
  run_signed_ops<int32e_t>("i32 operators", -2000000, 13);
  run_signed_ops<int64e_t>("i64 operators", -9000000000ll, 17ll);

  run_fp_ops<fp32e_t>("fp32 operators", 6.5f, 2.0f);
  run_fp_ops<fp64e_t>("fp64 operators", 10.25, 4.0);

  CHECK_FP("support fabs(fp64e_t)", fabs(fp64e_t(-5.5)), 5.5);

  TEST_CAST(int8e_t, int16e_t, -7, "cast i8->i16");
  TEST_CAST(int8e_t, int32e_t, -7, "cast i8->i32");
  TEST_CAST(int8e_t, int64e_t, -7, "cast i8->i64");
  TEST_CAST(int8e_t, uint8e_t, -7, "cast i8->u8");
  TEST_CAST(int8e_t, uint16e_t, -7, "cast i8->u16");
  TEST_CAST(int8e_t, uint32e_t, -7, "cast i8->u32");
  TEST_CAST(int8e_t, uint64e_t, -7, "cast i8->u64");

  TEST_CAST(uint32e_t, int8e_t, 123, "cast u32->i8");
  TEST_CAST(uint32e_t, int16e_t, 123, "cast u32->i16");
  TEST_CAST(uint32e_t, int32e_t, 123, "cast u32->i32");
  TEST_CAST(uint32e_t, int64e_t, 123, "cast u32->i64");
  TEST_CAST(uint32e_t, uint8e_t, 123, "cast u32->u8");
  TEST_CAST(uint32e_t, uint16e_t, 123, "cast u32->u16");
  TEST_CAST(uint32e_t, uint64e_t, 123, "cast u32->u64");

  TEST_CAST(int32e_t, fp32e_t, 123, "cast i32->fp32");
  TEST_CAST(int32e_t, fp64e_t, 123, "cast i32->fp64");
  TEST_CAST(uint32e_t, fp32e_t, 123, "cast u32->fp32");
  TEST_CAST(uint32e_t, fp64e_t, 123, "cast u32->fp64");

  TEST_CAST(fp32e_t, int8e_t, 42.0f, "cast fp32->i8");
  TEST_CAST(fp32e_t, int16e_t, 42.0f, "cast fp32->i16");
  TEST_CAST(fp32e_t, int32e_t, 42.0f, "cast fp32->i32");
  TEST_CAST(fp32e_t, int64e_t, 42.0f, "cast fp32->i64");
  TEST_CAST(fp32e_t, uint8e_t, 42.0f, "cast fp32->u8");
  TEST_CAST(fp32e_t, uint16e_t, 42.0f, "cast fp32->u16");
  TEST_CAST(fp32e_t, uint32e_t, 42.0f, "cast fp32->u32");
  TEST_CAST(fp32e_t, uint64e_t, 42.0f, "cast fp32->u64");

  TEST_CAST(fp64e_t, int8e_t, 55.0, "cast fp64->i8");
  TEST_CAST(fp64e_t, int16e_t, 55.0, "cast fp64->i16");
  TEST_CAST(fp64e_t, int32e_t, 55.0, "cast fp64->i32");
  TEST_CAST(fp64e_t, int64e_t, 55.0, "cast fp64->i64");
  TEST_CAST(fp64e_t, uint8e_t, 55.0, "cast fp64->u8");
  TEST_CAST(fp64e_t, uint16e_t, 55.0, "cast fp64->u16");
  TEST_CAST(fp64e_t, uint32e_t, 55.0, "cast fp64->u32");
  TEST_CAST(fp64e_t, uint64e_t, 55.0, "cast fp64->u64");

  if (g_failures != 0)
  {
    libmin_printf("ERROR: %d tests failed.\n", g_failures);
    return -1;
  }

  libmin_printf("INFO: all encrypted type tests passed.\n");
  libmin_success();
  return 0;
}
