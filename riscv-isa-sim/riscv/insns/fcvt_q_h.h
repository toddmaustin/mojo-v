require_extension(EXT_ZFHMIN);
require_extension('Q');
require_fp;
softfloat_roundingMode = RM;
WRITE_FRD(f16_to_f128(f16(FRS1)));

if (!FP_SECREG_REF(insn.rd()))
  set_fp_exceptions; // not a secret reg write, declare exceptions
else
  drop_fp_exceptions; // else, drop the exception
