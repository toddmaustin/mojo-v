int csr = validate_csr(insn.csr(), true);

if (SECREG_REF(insn.rs1()))
{ 
  // cannot use secret register source
  throw trap_security_exception(insn.bits());
}

reg_t old = p->get_csr(csr, insn, true);
p->put_csr(csr, RS1);
WRITE_RD(sext_xlen(old));
serialize();
