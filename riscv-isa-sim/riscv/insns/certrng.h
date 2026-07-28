require_extension(EXT_ZKMOJOV);

// The I-format rs1 field is reserved and must be zero; imm[11:0] is site_id.
// WRITE_RD installs the normal opcode-derived leaf label.  The sampled bits
// are deliberately not registered as a dfhash input.
if (!p->get_secreg_mode()
    || !p->get_state()->mojov_dcvalid
    || !IS_SECREG(insn.rd())
    || insn.rs1() != 0)
  throw trap_security_exception(insn.bits());

WRITE_RD(p->mojov_trng64());
