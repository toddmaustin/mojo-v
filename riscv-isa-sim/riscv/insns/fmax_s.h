require_either_extension('F', EXT_ZFINX);
require_fp;
// Mojo-V: make this insn implementation idempotent
auto frs1_f = FRS1_F;
auto frs2_f = FRS2_F;
bool greater = f32_lt_quiet(frs2_f, frs1_f) ||
               (f32_eq(frs2_f, frs1_f) && (frs2_f.v & F32_SIGN));
if (isNaNF32UI(frs1_f.v) && isNaNF32UI(frs2_f.v))
  WRITE_FRD_F(f32(defaultNaNF32UI));
else
  WRITE_FRD_F((greater || isNaNF32UI(frs2_f.v) ? frs1_f : frs2_f));

if (!FP_SECREG_REF(insn.rd()))
  set_fp_exceptions; // not a secret reg write, declare exceptions
else
  drop_fp_exceptions; // else, drop the exception
