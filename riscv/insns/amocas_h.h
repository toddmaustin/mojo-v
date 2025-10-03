require_extension(EXT_ZACAS);
require_extension(EXT_ZABHA);

// Mojo-V: RS2 finds its way (via swap operation) to unencrypted memory
if (p->extension_enabled(EXT_ZKMOJOV) && p->get_secreg_mode() && IS_SECREG(insn.rs2()))
{
  // illegal use of SDE
  throw trap_illegal_instruction(insn.bits());
}

WRITE_RD(sreg_t(MMU.amo_compare_and_swap<int16_t>(BASE_RS1, RD, RS2)));
