require_extension(EXT_ZFH);
require_extension(EXT_ZFA);
require_fp;

auto __frs1_h = (FRS1_H);
auto __frs2_h = (FRS2_H);

bool greater = f16_lt_quiet(__frs2_h, __frs1_h) ||
               (f16_eq(__frs2_h, __frs1_h) && (__frs2_h.v & F16_SIGN));
if (isNaNF16UI(__frs1_h.v) || isNaNF16UI(__frs2_h.v))
  WRITE_FRD_H(f16(defaultNaNF16UI));
else
  WRITE_FRD_H(greater ? __frs1_h : __frs2_h);

if (!FP_SECREG_REF(insn.rd()))
  set_fp_exceptions; // not a secret reg write, declare exceptions
else
  drop_fp_exceptions; // else, drop the exception
