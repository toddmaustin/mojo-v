require_extension('M');

auto __rs1 = (RS1);
auto __rs2 = (RS2);

reg_t lhs = zext_xlen(__rs1);
reg_t rhs = zext_xlen(__rs2);
if (rhs == 0)
  WRITE_RD(sext_xlen(__rs1));
else
  WRITE_RD(sext_xlen(lhs % rhs));
