require_extension('F');
require_fp;

// Mojo-V: RS2 finds its way to unencrypted memory
if (FP_SECREG_REF(insn.rs2()))
{
  // illegal use of SDE
  throw trap_security_exception(insn.bits());
}

MMU.store<uint32_t>(BASE_RS1 + insn.s_imm(), FRS2.v[0]);
