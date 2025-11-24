require_extension(EXT_ZBB);

auto __rs1 = (RS1);
auto __rs2 = (RS2);

WRITE_RD(sext_xlen(__rs1 < __rs2 ? __rs1 : __rs2));
