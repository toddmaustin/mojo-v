require_extension('Q');
require_extension(EXT_ZFA);
require_fp;
WRITE_FRD(f128_roundToInt(f128(FRS1), RM, true));

if (!FP_SECREG_REF(insn.rd()))
  set_fp_exceptions; // not a secret reg write, declare exceptions
else
  drop_fp_exceptions; // else, drop the exception
