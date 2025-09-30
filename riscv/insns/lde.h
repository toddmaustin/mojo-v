require_extension(EXT_ZKMOJOV);

WRITE_RD(MMU.load<int64_t>(BASE_RS1 + insn.i_imm()));

