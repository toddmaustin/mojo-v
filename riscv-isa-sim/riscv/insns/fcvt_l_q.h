require_extension('Q');
require_rv64;
require_fp;
softfloat_roundingMode = RM;
WRITE_RD(f128_to_i64(f128(FRS1), RM, true));

if (!SECREG_REF(insn.rd()))
  set_fp_exceptions; // not a secret reg write, declare exceptions
else
  drop_fp_exceptions; // else, drop the exception
