require_extension('S');
require_impl(IMPL_MMU);

// Spike doesn't implement address specific TLB shootdowns, but still prevent secret regs
if (SECREG_REF(insn.rs1()) || SECREG_REF(insn.rs2()))
{
  // illegal use of RS1/RS2 address
  throw trap_security_exception(insn.bits());
}

if (STATE.v) {
  if (STATE.prv == PRV_U || get_field(STATE.hstatus->read(), HSTATUS_VTVM))
    require_novirt();
} else {
  require_privilege(get_field(STATE.mstatus->read(), MSTATUS_TVM) ? PRV_M : PRV_S);
}
MMU.flush_tlb();
