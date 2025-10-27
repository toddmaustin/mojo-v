require_extension(EXT_ZABHA);

// Mojo-V: RS2 finds its way (via swap operation) to unencrypted memory
if (SECREG_REF(insn.rs2()))
{
  // illegal use of SDE
  throw trap_security_exception(insn.bits());
}

WRITE_RD(sreg_t(MMU.amo<int8_t>(BASE_RS1, [&](int8_t UNUSED lhs) { return RS2; })));
