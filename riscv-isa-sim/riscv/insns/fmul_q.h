require_extension('Q');
require_fp;
softfloat_roundingMode = RM;
WRITE_FRD(f128_mul(f128(FRS1), f128(FRS2)));

if (!FP_SECREG_REF(insn.rd()))
  set_fp_exceptions; // not a secret reg write, declare exceptions
else
  drop_fp_exceptions; // else, drop the exception
