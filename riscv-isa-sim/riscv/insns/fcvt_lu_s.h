require_either_extension('F', EXT_ZFINX);
require_rv64;
require_fp;
softfloat_roundingMode = RM;
WRITE_RD(f32_to_ui64(FRS1_F, RM, true));

if (!SECREG_REF(insn.rd()))
  set_fp_exceptions; // not a secret reg write, declare exceptions
else
  drop_fp_exceptions; // else, drop the exception
