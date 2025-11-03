require_either_extension('D', EXT_ZDINX);
require_rv64;
require_fp;
softfloat_roundingMode = RM;
WRITE_RD(f64_to_i64(FRS1_D, RM, true));

if (!SECREG_REF(insn.rd()))
  set_fp_exceptions; // not a secret reg write, declare exceptions
else
  drop_fp_exceptions; // else, drop the exception
