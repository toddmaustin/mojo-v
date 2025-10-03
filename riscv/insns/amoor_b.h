require_extension(EXT_ZABHA);

// Mojo-V: RS2 finds its way (via OR operation) to unencrypted memory
if (p->extension_enabled(EXT_ZKMOJOV) && p->get_secreg_mode() && IS_SECREG(insn.rs2()))
{
  // illegal use of SDE
  throw trap_illegal_instruction(insn.bits());
}

WRITE_RD(sreg_t(MMU.amo<int8_t>(BASE_RS1, [&](int8_t lhs) { return lhs | RS2; })));
