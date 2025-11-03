require_either_extension(EXT_ZFH, EXT_ZHINX);
require_fp;
WRITE_FRD_H(f16_min(FRS1_H, FRS2_H));

if (!FP_SECREG_REF(insn.rd()))
  set_fp_exceptions; // not a secret reg write, declare exceptions
else
  drop_fp_exceptions; // else, drop the exception
