require_extension(EXT_INTERNAL_ZFH_MOVE);
require_fp;
WRITE_FRD(f16(MMU.load<uint16_t>(BASE_RS1 + insn.i_imm())));
