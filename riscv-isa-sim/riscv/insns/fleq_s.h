require_extension('F');
require_extension(EXT_ZFA);
require_fp;
WRITE_RD(f32_le_quiet(FRS1_F, FRS2_F));

if (!SECREG_REF(insn.rd()))
  set_fp_exceptions; // not a secret reg write, declare exceptions
else
  drop_fp_exceptions; // else, drop the exception
