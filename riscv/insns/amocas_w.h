require_extension(EXT_ZACAS);

// Mojo-V: RS2 finds its way to unencrypted memory
if (p->extension_enabled(EXT_ZKMOJOV) && p->get_secreg_mode() && IS_SECREG(insn.rs2()))
{
  // illegal use of store operation
  throw trap_illegal_instruction(insn.bits());
}

WRITE_RD(sext32(MMU.amo_compare_and_swap<uint32_t>(BASE_RS1, RD, RS2)));
