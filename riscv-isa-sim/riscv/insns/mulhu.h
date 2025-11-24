require_either_extension('M', EXT_ZMMUL);

auto __rs1 = (RS1);
auto __rs2 = (RS2);

if (xlen == 64)
  WRITE_RD(mulhu(__rs1, __rs2));
else
  WRITE_RD(sext32(((uint64_t)(uint32_t)__rs1 * (uint64_t)(uint32_t)__rs2) >> 32));
