require_extension('H');
require_novirt();
require_privilege(get_field(STATE.hstatus->read(), HSTATUS_HU) ? PRV_U : PRV_S);

// Mojo-V: RS2 finds its way to unencrypted memory
if (SECREG_REF(insn.rs2()))
{
  // illegal use of SDE
  throw trap_security_exception(insn.bits());
}

MMU.guest_store<uint16_t>(BASE_RS1, RS2);
