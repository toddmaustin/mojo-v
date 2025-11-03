require_extension(EXT_ZFH);
require_extension(EXT_ZFA);
require_fp;
bool greater = f16_lt_quiet(FRS2_H, FRS1_H) ||
               (f16_eq(FRS2_H, FRS1_H) && (FRS2_H.v & F16_SIGN));
if (isNaNF16UI(FRS1_H.v) || isNaNF16UI(FRS2_H.v))
  WRITE_FRD_H(f16(defaultNaNF16UI));
else
  WRITE_FRD_H(greater ? FRS1_H : FRS2_H);

if (!FP_SECREG_REF(insn.rd()))
  set_fp_exceptions; // not a secret reg write, declare exceptions
else
  drop_fp_exceptions; // else, drop the exception
