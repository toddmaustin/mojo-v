require_either_extension('F', EXT_ZFINX);
require_fp;
WRITE_RD(f32_le(FRS1_F, FRS2_F));

if (!SECREG_REF(insn.rd()))
  set_fp_exceptions; // not a secret reg write, declare exceptions
else
  drop_fp_exceptions; // else, drop the exception
