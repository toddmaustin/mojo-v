require_either_extension('D', EXT_ZDINX);
require_fp;
WRITE_RD(f64_le(FRS1_D, FRS2_D));

if (!SECREG_REF(insn.rd()))
  set_fp_exceptions; // not a secret reg write, declare exceptions
else
  drop_fp_exceptions; // else, drop the exception
