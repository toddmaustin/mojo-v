require_extension(EXT_ZFBFMIN);
require_fp;
softfloat_roundingMode = RM;
WRITE_FRD_BF(f32_to_bf16(FRS1_F));

if (!FP_SECREG_REF(insn.rd()))
  set_fp_exceptions; // not a secret reg write, declare exceptions
else
  drop_fp_exceptions; // else, drop the exception
