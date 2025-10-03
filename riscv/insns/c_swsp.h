require_extension(EXT_ZCA);

// Mojo-V: RS2 finds its way to unencrypted memory
if (p->extension_enabled(EXT_ZKMOJOV) && p->get_secreg_mode() && IS_SECREG(insn.rvc_rs2()))
{
  // illegal use of SDE
  throw trap_illegal_instruction(insn.bits());
}

MMU.store<uint32_t>(BASE_RVC_SP + insn.rvc_swsp_imm(), RVC_RS2);
