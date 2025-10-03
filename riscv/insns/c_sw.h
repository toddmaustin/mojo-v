require_extension(EXT_ZCA);

// Mojo-V: RS2 finds its way to unencrypted memory
if (p->extension_enabled(EXT_ZKMOJOV) && p->get_secreg_mode() && IS_SECREG(insn.rvc_rs2s()))
{
  // illegal use of SDE
  throw trap_illegal_instruction(insn.bits());
}

MMU.store<uint32_t>(BASE_RVC_RS1S + insn.rvc_lw_imm(), RVC_RS2S);
