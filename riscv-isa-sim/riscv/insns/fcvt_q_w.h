require_extension('Q');
require_fp;
softfloat_roundingMode = RM;
WRITE_FRD(i32_to_f128((int32_t)RS1));

if (!FP_SECREG_REF(insn.rd()))
  set_fp_exceptions; // not a secret reg write, declare exceptions
else
  drop_fp_exceptions; // else, drop the exception
