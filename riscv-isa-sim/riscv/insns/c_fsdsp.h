require_extension(EXT_ZCD);
require_fp;

// Mojo-V: RS2 finds its way to unencrypted memory
if (FP_SECREG_REF(insn.rvc_rs2()))
{
  // illegal use of SDE
  throw trap_security_exception(insn.bits());
}

MMU.store<uint64_t>(BASE_RVC_SP + insn.rvc_sdsp_imm(), RVC_FRS2.v[0]);
