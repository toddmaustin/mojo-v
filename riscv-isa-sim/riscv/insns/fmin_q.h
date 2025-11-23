require_extension('Q');
require_fp;

auto __frs1 = (FRS1);
auto __frs2 = (FRS2);

bool less = f128_lt_quiet(f128(__frs1), f128(__frs2)) ||
            (f128_eq(f128(__frs1), f128(__frs2)) && (f128(__frs1).v[1] & F64_SIGN));
if (isNaNF128(f128(__frs1)) && isNaNF128(f128(__frs2)))
  WRITE_FRD(f128(defaultNaNF128()));
else
  WRITE_FRD(less || isNaNF128(f128(__frs2)) ? __frs1 : __frs2);

if (!FP_SECREG_REF(insn.rd()))
  set_fp_exceptions; // not a secret reg write, declare exceptions
else
  drop_fp_exceptions; // else, drop the exception
