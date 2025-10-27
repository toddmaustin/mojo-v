require_extension(EXT_ZCA);
require((xlen == 64) || p->extension_enabled(EXT_ZCLSD));

// Mojo-V: RS2 finds its way to unencrypted memory
if (SECREG_REF(insn.rvc_rs2s()))
{ 
  // illegal use of store operation
  throw trap_security_exception(insn.bits());
}

if (xlen == 32) {
  MMU.store<uint64_t>(BASE_RVC_RS1S + insn.rvc_ld_imm(), RVC_RS2S_PAIR);
} else {
  MMU.store<uint64_t>(BASE_RVC_RS1S + insn.rvc_ld_imm(), RVC_RS2S);
}
