require_extension(EXT_ZKMOJOV);

// Mojo-V: SDE requires SECREGs enabled AND RS2 must be secret reg
if (!p->get_secreg_mode() || !IS_SECREG(insn.rs2())) 
{
  // illegal use of SDE
  throw trap_security_exception(insn.bits());
}

union mojov_memfmt_t ctval;
union mojov_memfmt_t ptval;

// Mojo-V: prep the encrypted packet with RS2 value, salt and sig
ptval.pt = { RS2, (uint32_t)rand(), MOJOV_PT_SIG };

// encrypt the memory packet with the processor's internal key
simon_128_128_encrypt(&p->simon_state, ptval.ct, &ctval.ct);

// Mojo-V: all good, store 3rd-party encrypted value to memory
MMU.store<uint128_t>(BASE_RS1 + insn.s_imm(), ctval.ct);

