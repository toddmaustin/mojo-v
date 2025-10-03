require_extension(EXT_ZABHA);

// Mojo-V: RS2 finds its way (via min operation) to unencrypted memory
if (p->extension_enabled(EXT_ZKMOJOV) && p->get_secreg_mode() && IS_SECREG(insn.rs2()))
{
  // illegal use of SDE
  throw trap_illegal_instruction(insn.bits());
}

WRITE_RD(sreg_t(MMU.amo<int16_t>(BASE_RS1, [&](uint16_t lhs) { return std::min(lhs, uint16_t(RS2)); })));
