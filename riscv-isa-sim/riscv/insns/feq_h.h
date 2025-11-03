require_either_extension(EXT_ZFH, EXT_ZHINX);
require_fp;
WRITE_RD(f16_eq(FRS1_H, FRS2_H));

if (!SECREG_REF(insn.rd()))
  set_fp_exceptions; // not a secret reg write, declare exceptions
else
  drop_fp_exceptions; // else, drop the exception
