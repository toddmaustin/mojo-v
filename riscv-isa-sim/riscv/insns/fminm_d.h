require_extension('D');
require_extension(EXT_ZFA);
require_fp;

auto __frs1_d = (FRS1_D);
auto __frs2_d = (FRS2_D);

bool less = f64_lt_quiet(__frs1_d, __frs2_d) ||
            (f64_eq(__frs2_d, __frs1_d) && (__frs1_d.v & F64_SIGN));
if (isNaNF64UI(__frs1_d.v) || isNaNF64UI(__frs2_d.v))
  WRITE_FRD_D(f64(defaultNaNF64UI));
else
  WRITE_FRD_D(less ? __frs1_d : __frs2_d);

if (!FP_SECREG_REF(insn.rd()))
  set_fp_exceptions; // not a secret reg write, declare exceptions
else
  drop_fp_exceptions; // else, drop the exception
