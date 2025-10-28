require_extension(EXT_ZICFISS);
require_extension('A');

DECLARE_XENVCFG_VARS(SSE);
require_envcfg(SSE);

// Mojo-V: RS2 finds its way (via swap operation) to unencrypted memory
if (SECREG_REF(insn.rs2()))
{
  // illegal use of SDE
  throw trap_security_exception(insn.bits());
}

WRITE_RD(sext32(MMU.ssamoswap<uint32_t>(BASE_RS1, RS2)));

