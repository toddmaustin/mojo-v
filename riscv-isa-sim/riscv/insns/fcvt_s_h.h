require_either_extension(EXT_ZFHMIN, EXT_ZHINXMIN);
require_fp;
softfloat_roundingMode = RM;
WRITE_FRD_F(f16_to_f32(FRS1_H));

if (!FP_SECREG_REF(insn.rd()))
  set_fp_exceptions; // not a secret reg write, declare exceptions
else
  drop_fp_exceptions; // else, drop the exception
