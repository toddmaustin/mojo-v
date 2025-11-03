require_extension('Q');
require_fp;
WRITE_RD(f128_eq(f128(FRS1), f128(FRS2)));

if (!SECREG_REF(insn.rd()))
  set_fp_exceptions; // not a secret reg write, declare exceptions
else
  drop_fp_exceptions; // else, drop the exception
