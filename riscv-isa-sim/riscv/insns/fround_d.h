require_extension('D');
require_extension(EXT_ZFA);
require_fp;
WRITE_FRD_D(f64_roundToInt(FRS1_D, RM, false));

if (!FP_SECREG_REF(insn.rd()))
  set_fp_exceptions; // not a secret reg write, declare exceptions
else
  drop_fp_exceptions; // else, drop the exception
