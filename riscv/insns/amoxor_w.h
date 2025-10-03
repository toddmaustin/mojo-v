require_extension('A');

// Mojo-V: RS2 finds its way (via XOR operation) to unencrypted memory
if (p->extension_enabled(EXT_ZKMOJOV) && p->get_secreg_mode() && IS_SECREG(insn.rs2()))
{
  // illegal use of SDE
  throw trap_illegal_instruction(insn.bits());
}

WRITE_RD(sext32(MMU.amo<uint32_t>(BASE_RS1, [&](uint32_t lhs) { return lhs ^ RS2; })));
