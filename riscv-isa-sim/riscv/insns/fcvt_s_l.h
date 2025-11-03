require_either_extension('F', EXT_ZFINX);
require_rv64;
require_fp;
softfloat_roundingMode = RM;
WRITE_FRD_F(i64_to_f32(RS1));

if (!FP_SECREG_REF(insn.rd()))
  set_fp_exceptions; // not a secret reg write, declare exceptions
else
  drop_fp_exceptions; // else, drop the exception
