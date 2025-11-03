require_extension(EXT_INTERNAL_ZFH_MOVE);
require_fp;

// Mojo-V: RS2 finds its way to unencrypted memory
if (FP_SECREG_REF(insn.rs2()))
{
  // illegal use of SDE
  throw trap_security_exception(insn.bits());
}

MMU.store<uint16_t>(BASE_RS1 + insn.s_imm(), FRS2.v[0]);
