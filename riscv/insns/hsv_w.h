require_extension('H');
require_novirt();

// Mojo-V: RS2 finds its way to unencrypted memory
if (SECREG_REF(insn.rs2()))
{
  // illegal use of SDE
  throw trap_security_exception(insn.bits());
}

require_privilege(get_field(STATE.hstatus->read(), HSTATUS_HU) ? PRV_U : PRV_S);
MMU.guest_store<uint32_t>(BASE_RS1, RS2);
