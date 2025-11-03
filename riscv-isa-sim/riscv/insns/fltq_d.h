require_extension('D');
require_extension(EXT_ZFA);
require_fp;
WRITE_RD(f64_lt_quiet(FRS1_D, FRS2_D));

if (!SECREG_REF(insn.rd()))
  set_fp_exceptions; // not a secret reg write, declare exceptions
else
  drop_fp_exceptions; // else, drop the exception
