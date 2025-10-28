require_extension('A');

// Mojo-V: RS2 finds its way (via OR operation) to unencrypted memory
if (SECREG_REF(insn.rs2()))
{
  // illegal use of SDE
  throw trap_security_exception(insn.bits());
}

WRITE_RD(sext32(MMU.amo<uint32_t>(BASE_RS1, [&](uint32_t lhs) { return lhs | RS2; })));
