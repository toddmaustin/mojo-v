require_extension(EXT_ZCA);

// Mojo-V: RS2 finds its way to unencrypted memory
if (SECREG_REF(insn.rvc_rs2s()))
{
  // illegal use of SDE
  throw trap_security_exception(insn.bits());
}

MMU.store<uint32_t>(BASE_RVC_RS1S + insn.rvc_lw_imm(), RVC_RS2S);
