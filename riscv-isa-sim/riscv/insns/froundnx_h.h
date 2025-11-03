require_extension(EXT_ZFH);
require_extension(EXT_ZFA);
require_fp;
WRITE_FRD_H(f16_roundToInt(FRS1_H, RM, true));

if (!FP_SECREG_REF(insn.rd()))
  set_fp_exceptions; // not a secret reg write, declare exceptions
else
  drop_fp_exceptions; // else, drop the exception
