require_extension(EXT_ZACAS);

// Mojo-V: RS2 finds its way to unencrypted memory
if (SECREG_REF(insn.rs2()))
{
  // illegal use of store operation
  throw trap_security_exception(insn.bits());
}

WRITE_RD(sext32(MMU.amo_compare_and_swap<uint32_t>(BASE_RS1, RD, RS2)));
