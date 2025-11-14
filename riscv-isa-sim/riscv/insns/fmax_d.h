require_either_extension('D', EXT_ZDINX);
require_fp;
// Mojo-V: make this insn implementation idempotent
auto frs1_d = FRS1_D;
auto frs2_d = FRS2_D;
bool greater = f64_lt_quiet(frs2_d, frs1_d) ||
              (f64_eq(frs2_d, frs1_d) && (frs2_d.v & F64_SIGN));
if (isNaNF64UI(frs1_d.v) && isNaNF64UI(frs2_d.v))
  WRITE_FRD_D(f64(defaultNaNF64UI));
else
  WRITE_FRD_D((greater || isNaNF64UI(frs2_d.v) ? frs1_d : frs2_d));

if (!FP_SECREG_REF(insn.rd()))
  set_fp_exceptions; // not a secret reg write, declare exceptions
else
  drop_fp_exceptions; // else, drop the exception
