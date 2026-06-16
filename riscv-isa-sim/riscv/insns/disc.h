require_extension(EXT_ZKMOJOV);

// Mojo-V: DISC is the only integer operation that may consume a datagrant.
if (!p->get_secreg_mode()
    || SECREG_CSR_FIELD(MSECREGCFG_FORMAT_SEL) != FORMAT_SEL_PROOFCARRYING
    || !IS_SECREG(insn.rs1())
    || IS_SECREG(insn.rd())
    || !p->get_state()->datagrant_xpr[insn.rs2()])
{
  throw trap_security_exception(insn.bits());
}

if (p->get_state()->dfhash_xpr[insn.rs1()] != p->get_state()->XPR[insn.rs2()])
{
  throw trap_security_exception(insn.bits());
}

WRITE_RD(p->get_state()->XPR[insn.rs1()]);
