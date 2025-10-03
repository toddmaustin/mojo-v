require_extension(EXT_ZCA);
require((xlen == 64) || p->extension_enabled(EXT_ZCLSD));

// Mojo-V: RS2 finds its way to unencrypted memory
if (p->extension_enabled(EXT_ZKMOJOV) && p->get_secreg_mode() && IS_SECREG(insn.rvc_rs2s()))
{ 
  // illegal use of store operation
  throw trap_illegal_instruction(insn.bits());
}

if (xlen == 32) {
  MMU.store<uint64_t>(BASE_RVC_RS1S + insn.rvc_ld_imm(), RVC_RS2S_PAIR);
} else {
  MMU.store<uint64_t>(BASE_RVC_RS1S + insn.rvc_ld_imm(), RVC_RS2S);
}
