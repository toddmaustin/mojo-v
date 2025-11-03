require_extension(EXT_ZFH);
require_extension(EXT_ZFA);
require_fp;
WRITE_RD(f16_le_quiet(FRS1_H, FRS2_H));

if (!SECREG_REF(insn.rd()))
  set_fp_exceptions; // not a secret reg write, declare exceptions
else
  drop_fp_exceptions; // else, drop the exception
