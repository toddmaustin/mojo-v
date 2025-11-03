require_either_extension(EXT_ZFH, EXT_ZHINX);
require_rv64;
require_fp;
softfloat_roundingMode = RM;
WRITE_FRD_H(ui64_to_f16(RS1));

if (!FP_SECREG_REF(insn.rd()))
  set_fp_exceptions; // not a secret reg write, declare exceptions
else
  drop_fp_exceptions; // else, drop the exception
