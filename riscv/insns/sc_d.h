require_extension('A');
require_rv64;

// Mojo-V: RS2 finds its way to unencrypted memory
if (p->extension_enabled(EXT_ZKMOJOV) && p->get_secreg_mode() && IS_SECREG(insn.rs2()))
{ 
  // illegal use of store operation
  throw trap_illegal_instruction(insn.bits());
}

bool have_reservation = MMU.store_conditional<uint64_t>(BASE_RS1, RS2);

WRITE_RD(!have_reservation);
