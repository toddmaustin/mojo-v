require_extension('H');
require_novirt();
require_privilege(PRV_S);

// Spike doesn't implement address specific TLB shootdowns, but still prevent secret regs
if (SECREG_REF(insn.rs1()) || SECREG_REF(insn.rs2()))
{
  // illegal use of RS1/RS2 address
  throw trap_security_exception(insn.bits());
}

MMU.flush_tlb();
