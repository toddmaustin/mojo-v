bool write = insn.rs1() != 0;
int csr = validate_csr(insn.csr(), write);

if (SECREG_REF(insn.rs1()))
{
  // cannot use secret register source RS1
  throw trap_security_exception(insn.bits());
}

reg_t old = p->get_csr(csr, insn, write);
if (write) {
  p->put_csr(csr, old | RS1);
}
WRITE_RD(sext_xlen(old));
serialize();
