require_extension(EXT_ZKMOJOV);

MMU.store<uint64_t>(BASE_RS1 + insn.s_imm(), RS2);

