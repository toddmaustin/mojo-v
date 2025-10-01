require_extension(EXT_ZKMOJOV);

// Mojo-V: LDE requires SECREGs enabled AND RD must be secret reg
if (!p->get_secreg_mode() || !IS_SECREG(insn.rd()))
{
  // illegal use of LDE
  throw trap_illegal_instruction(insn.bits());
}

// Mojo-V: read 3rd-party decrypted value into SECREG RD
WRITE_RD(MMU.load<int64_t>(BASE_RS1 + insn.i_imm()));

