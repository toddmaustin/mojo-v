require_extension('F');
require_fp;
MMU.store<uint32_t>(BASE_RS1 + insn.s_imm(), FRS2.v[0]);
