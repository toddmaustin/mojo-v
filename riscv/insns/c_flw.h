require_extension(EXT_ZCF);
require_fp;
WRITE_RVC_FRS2S(f32(MMU.load<uint32_t>(BASE_RVC_RS1S + insn.rvc_lw_imm())));
