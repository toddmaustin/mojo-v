require_extension(EXT_ZKMOJOV);

// Mojo-V: FDISC is the only floating-point operation that may consume a datagrant.
if (!p->get_secreg_mode()
    || SECREG_CSR_FIELD(MSECREGCFG_FORMAT_SEL) != FORMAT_SEL_PROOFCARRYING
    || !IS_FP_SECREG(insn.rs1())
    || IS_SECREG(insn.rd())
    || !p->get_state()->datagrant_xpr[insn.rs2()])
{
  throw trap_security_exception(insn.bits());
}

if (p->get_state()->dfhash_fpr[insn.rs1()] != p->get_state()->XPR[insn.rs2()])
{
  throw trap_security_exception(insn.bits());
}

WRITE_RD(p->get_state()->FPR[insn.rs1()].v[0]);
