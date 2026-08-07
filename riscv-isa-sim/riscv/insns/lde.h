require_rv64;
require_extension(EXT_ZKMOJOV);

// Mojo-V: LDE requires SECREGs enabled AND RD must be secret reg
if (!p->get_secreg_mode() || !IS_SECREG(insn.rd()))
{
  // illegal use of LDE
  throw trap_security_exception(insn.bits());
}

// determine if it is a fast or strong memory format
if (SECREG_CSR_FIELD(MSECREGCFG_FORMAT_SEL) == FORMAT_SEL_FAST)
{
  union mojov_mem_fast_t ctval;
  union mojov_mem_fast_t ptval;

  // Mojo-V: read 3rd-party encrypted ciphertext into MEMVAL
  ctval.ct = MMU.load<uint128_t>(BASE_RS1 + insn.i_imm());

  // decrypt the value with the processor-internal key
  simon_128_128_decrypt(&p->simon_state, ctval.ct, &ptval.ct);

  if (ptval.pt.auth_sig != (uint32_t)p->get_state()->mojov_dc.contract_sig)
  {
    // Mojo-V not valid ciphertext, trap out...
    throw trap_security_exception(insn.bits());
  }

  // Mojo-V: all good, write the decrypted 3rd-party value to the secret register
  WRITE_RD(ptval.pt.val);
}
else if (SECREG_CSR_FIELD(MSECREGCFG_FORMAT_SEL) == FORMAT_SEL_STRONG)
{
  union mojov_mem_strong_t ctval;
  union mojov_mem_strong_t ptval;

  // Mojo-V: read 3rd-party encrypted ciphertext into MEMVAL
  auto __base = (BASE_RS1);
  ctval.ct.ct_lo = MMU.load<uint128_t>(__base + insn.i_imm());
  ctval.ct.ct_hi = MMU.load<uint128_t>((__base + insn.i_imm()) + 16);

  // decrypt the value with the processor-internal key
  simon_128_128_decrypt(&p->simon_state, ctval.ct.ct_lo, &ptval.ct.ct_lo);
  simon_128_128_decrypt(&p->simon_state, ctval.ct.ct_hi, &ptval.ct.ct_hi);
  ptval.ct.ct_hi = ptval.ct.ct_hi ^ ctval.ct.ct_lo;

  if (ptval.pt.auth_sig != p->get_state()->mojov_dc.contract_sig)
  {
    // Mojo-V not valid ciphertext, trap out...
    throw trap_security_exception(insn.bits());
  }

  // fprintf(stderr, "LDE: val = %lu, sig = 0x%08lx\n", ptval.pt.val, ptval.pt.auth_sig);

  // Mojo-V: all good, write the decrypted 3rd-party value to the secret register
  WRITE_RD(ptval.pt.val);
}
else if (SECREG_CSR_FIELD(MSECREGCFG_FORMAT_SEL) == FORMAT_SEL_PROOFCARRYING)
{
  union mojov_mem_proofcarrying_t ctval;
  union mojov_mem_proofcarrying_t ptval;

  // Mojo-V: read 3rd-party encrypted ciphertext into MEMVAL
  auto __base = (BASE_RS1);
  ctval.ct.ct_lo = MMU.load<uint128_t>(__base + insn.i_imm());
  ctval.ct.ct_hi = MMU.load<uint128_t>((__base + insn.i_imm()) + 16);

  // decrypt the value with the processor-internal key
  simon_128_128_decrypt(&p->simon_state, ctval.ct.ct_lo, &ptval.ct.ct_lo);
  simon_128_128_decrypt(&p->simon_state, ctval.ct.ct_hi, &ptval.ct.ct_hi);
  ptval.ct.ct_hi = ptval.ct.ct_hi ^ ctval.ct.ct_lo;

  const uint64_t contract_sig = p->get_state()->mojov_dc.contract_sig;
  const bool is_datagrant = (ptval.pt.auth_sig == contract_sig + 1);
  if (ptval.pt.auth_sig != contract_sig && !is_datagrant)
  {
    // Mojo-V not valid ciphertext or datagrant, trap out...
    throw trap_security_exception(insn.bits());
  }

  // Mojo-V: all good, write the decrypted 3rd-party value to the secret register.
  // Datagrants load their granted dfhash into the integer register value and
  // mark it so only DISC/FDISC may consume it.
  WRITE_RD(ptval.pt.val);
  p->get_state()->datagrant_xpr[insn.rd()] = is_datagrant;

  // Mojo-V: override the dfhash output, instead, take it from the decrypted packet
  p->get_state()->dfhash_xpr[insn.rd()] = is_datagrant ? 0 : ptval.pt.metadata;
}
else
{
  // illegal use of LDE
  throw trap_security_exception(insn.bits());
}

