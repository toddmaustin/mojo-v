require_extension('Q');
require_fp;
softfloat_roundingMode = RM;
WRITE_RD(sext32(f128_to_i32(f128(FRS1), RM, true)));

if (!SECREG_REF(insn.rd()))
  set_fp_exceptions; // not a secret reg write, declare exceptions
else
  drop_fp_exceptions; // else, drop the exception
