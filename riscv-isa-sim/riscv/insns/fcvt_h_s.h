require_either_extension(EXT_ZFHMIN, EXT_ZHINXMIN);
require_fp;
softfloat_roundingMode = RM;
WRITE_FRD_H(f32_to_f16(FRS1_F));

if (!FP_SECREG_REF(insn.rd()))
  set_fp_exceptions; // not a secret reg write, declare exceptions
else
  drop_fp_exceptions; // else, drop the exception
