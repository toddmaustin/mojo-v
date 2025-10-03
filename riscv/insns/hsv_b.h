require_extension('H');
require_novirt();
require_privilege(get_field(STATE.hstatus->read(), HSTATUS_HU) ? PRV_U : PRV_S);

// Mojo-V: RS2 finds its way to unencrypted memory
if (p->extension_enabled(EXT_ZKMOJOV) && p->get_secreg_mode() && IS_SECREG(insn.rs2()))
{
  // illegal use of SDE
  throw trap_illegal_instruction(insn.bits());
}

MMU.guest_store<uint8_t>(BASE_RS1, RS2);
