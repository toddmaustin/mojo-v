require_extension('F');
require_extension(EXT_ZFA);
require_fp;

auto __frs2_f = (FRS2_F);
auto __frs1_f = (FRS1_F);

bool greater = f32_lt_quiet(__frs2_f, __frs1_f) ||
               (f32_eq(__frs2_f, __frs1_f) && (__frs2_f.v & F32_SIGN));
if (isNaNF32UI(__frs1_f.v) || isNaNF32UI(__frs2_f.v))
  WRITE_FRD_F(f32(defaultNaNF32UI));
else
  WRITE_FRD_F(greater ? __frs1_f : __frs2_f);

if (!FP_SECREG_REF(insn.rd()))
  set_fp_exceptions; // not a secret reg write, declare exceptions
else
  drop_fp_exceptions; // else, drop the exception
