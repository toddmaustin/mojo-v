require_extension(EXT_ZICOND);

// Mojo-V: all operands MUST be evaluated
auto __rs1 = (RS1);
auto __rs2 = (RS2);

WRITE_RD(__rs2 == 0 ? 0 : __rs1);
