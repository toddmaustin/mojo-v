require_extension('F');
require_fp;
WRITE_FRD(f32(MMU.load<uint32_t>(BASE_RS1 + insn.i_imm())));
