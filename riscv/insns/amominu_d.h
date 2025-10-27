require_extension('A');
require_rv64;

// Mojo-V: RS2 finds its way (via MIN operation) to unencrypted memory
if (SECREG_REF(insn.rs2()))
{
  // illegal use of SDE
  throw trap_security_exception(insn.bits());
}

WRITE_RD(MMU.amo<uint64_t>(BASE_RS1, [&](uint64_t lhs) { return std::min(lhs, RS2); }));
