require_rv64;
WRITE_RD(MMU.load<uint32_t>(BASE_RS1 + insn.i_imm()));
