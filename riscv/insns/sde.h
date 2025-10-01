require_extension(EXT_ZKMOJOV);

// Mojo-V: SDE requires SECREGs enabled AND RS2 must be secret reg
if (!p->get_secreg_mode() || !IS_SECREG(insn.rs2())) 
{
  // illegal use of SDE
  throw trap_illegal_instruction(insn.bits());
}

// Mojo-V: store 3rd-party encrypted value from SECREG RS2

MMU.store<uint64_t>(BASE_RS1 + insn.s_imm(), RS2);

