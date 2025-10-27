require_extension('A');

// Mojo-V: RS2 finds its way (via swap operation) to unencrypted memory
if (SECREG_REF(insn.rs2()))
{
  // illegal use of SDE
  throw trap_security_exception(insn.bits());
}

WRITE_RD(sext32(MMU.amo<uint32_t>(BASE_RS1, [&](int32_t lhs) { return std::max(lhs, int32_t(RS2)); })));
