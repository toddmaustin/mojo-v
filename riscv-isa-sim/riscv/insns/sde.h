require_extension(EXT_ZKMOJOV);

// Mojo-V: SDE requires SECREGs enabled AND RS2 must be secret reg
if (!p->get_secreg_mode() || !IS_SECREG(insn.rs2())) 
{
  // illegal use of SDE
  throw trap_security_exception(insn.bits());
}

// determine if it is a fast or strong memory format
if (SECREG_CSR_FIELD(MSECREGCFG_FORMAT_SEL) == FORMAT_SEL_FAST)
{
  union mojov_mem_fast_t ctval;
  union mojov_mem_fast_t ptval;

  // Mojo-V: prep the encrypted packet with RS2 value, salt and signature
  const uint32_t expected_auth_sig = (uint32_t)p->get_state()->mojov_dc.contract_sig;
  ptval.pt = { RS2, (uint32_t)p->mojov_trng64(), expected_auth_sig };

  // encrypt the memory packet with the processor's internal key
  simon_128_128_encrypt(&p->simon_state, ptval.ct, &ctval.ct);

  // Mojo-V: all good, store 3rd-party encrypted value to memory
  MMU.store<uint128_t>(BASE_RS1 + insn.s_imm(), ctval.ct);
}
else if (SECREG_CSR_FIELD(MSECREGCFG_FORMAT_SEL) == FORMAT_SEL_STRONG)
{
  union mojov_mem_strong_t ctval;
  union mojov_mem_strong_t ptval;

  // Mojo-V: prep the encrypted packet with RS2 value, salt and sig
  ptval.pt = { RS2, p->mojov_trng64(), p->get_state()->mojov_dc.contract_sig, /* metadata */0 };

  // encrypt the memory packet with the processor's internal key
  simon_128_128_encrypt(&p->simon_state, ptval.ct.ct_lo, &ctval.ct.ct_lo);
  simon_128_128_encrypt(&p->simon_state, (ptval.ct.ct_hi ^ ctval.ct.ct_lo), &ctval.ct.ct_hi);

  // Mojo-V: all good, store 3rd-party encrypted value to memory
  auto __base = (BASE_RS1);
  MMU.store<uint128_t>(__base + insn.s_imm(), ctval.ct.ct_lo);
  MMU.store<uint128_t>((__base + insn.s_imm()) + 16, ctval.ct.ct_hi);
}
else if (SECREG_CSR_FIELD(MSECREGCFG_FORMAT_SEL) == FORMAT_SEL_PROOFCARRYING)
{
  union mojov_mem_proofcarrying_t ctval;
  union mojov_mem_proofcarrying_t ptval;

  // Mojo-V: prep the encrypted packet with RS2 value, salt and sig
  ptval.pt = { RS2, p->mojov_trng64(), p->get_state()->mojov_dc.contract_sig, /* metadata */p->get_state()->dfhash_xpr[insn.rs2()] };

  // encrypt the memory packet with the processor's internal key
  simon_128_128_encrypt(&p->simon_state, ptval.ct.ct_lo, &ctval.ct.ct_lo);
  simon_128_128_encrypt(&p->simon_state, (ptval.ct.ct_hi ^ ctval.ct.ct_lo), &ctval.ct.ct_hi);

  // Mojo-V: all good, store 3rd-party encrypted value to memory
  auto __base = (BASE_RS1);
  MMU.store<uint128_t>(__base + insn.s_imm(), ctval.ct.ct_lo);
  MMU.store<uint128_t>((__base + insn.s_imm()) + 16, ctval.ct.ct_hi);
}
else
{
  // illegal use of SDE
  throw trap_security_exception(insn.bits());
}
