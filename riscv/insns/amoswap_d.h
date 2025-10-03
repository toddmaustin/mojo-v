require_extension('A');
require_rv64;

// Mojo-V: RS2 finds its way (via swap operation) to unencrypted memory
if (p->extension_enabled(EXT_ZKMOJOV) && p->get_secreg_mode() && IS_SECREG(insn.rs2()))
{
  // illegal use of SDE
  throw trap_illegal_instruction(insn.bits());
}

WRITE_RD(MMU.amo<uint64_t>(BASE_RS1, [&](uint64_t UNUSED lhs) { return RS2; }));
