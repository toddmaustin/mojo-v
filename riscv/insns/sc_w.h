require_extension('A');

// Mojo-V: RS2 finds its way to unencrypted memory
if (p->extension_enabled(EXT_ZKMOJOV) && p->get_secreg_mode() && IS_SECREG(insn.rs2()))
{ 
  // illegal use of store operation
  throw trap_illegal_instruction(insn.bits());
}

bool have_reservation = MMU.store_conditional<uint32_t>(BASE_RS1, RS2);

WRITE_RD(!have_reservation);
