require_extension('F');
require_extension(EXT_ZFA);
require_fp;
WRITE_FRD_F(f32_roundToInt(FRS1_F, RM, true));

if (!FP_SECREG_REF(insn.rd()))
  set_fp_exceptions; // not a secret reg write, declare exceptions
else
  drop_fp_exceptions; // else, drop the exception
