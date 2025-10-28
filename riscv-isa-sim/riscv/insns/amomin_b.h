require_extension(EXT_ZABHA);

// Mojo-V: RS2 finds its way (via min operation) to unencrypted memory
if (SECREG_REF(insn.rs2()))
{
  // illegal use of SDE
  throw trap_security_exception(insn.bits());
}

WRITE_RD(sreg_t(MMU.amo<int8_t>(BASE_RS1, [&](int8_t lhs) { return std::min(lhs, int8_t(RS2)); })));
