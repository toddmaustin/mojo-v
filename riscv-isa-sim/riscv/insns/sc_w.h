require_extension('A');

// Mojo-V: RS2 finds its way to unencrypted memory
if (SECREG_REF(insn.rs2()))
{ 
  // illegal use of store operation
  throw trap_security_exception(insn.bits());
}

bool have_reservation = MMU.store_conditional<uint32_t>(BASE_RS1, RS2);

WRITE_RD(!have_reservation);
