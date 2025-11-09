require_extension(EXT_ZKMOJOV);

// Mojo-V: LDE requires SECREGs enabled AND RD must be secret reg
if (!p->get_secreg_mode() || !IS_SECREG(insn.rd()))
{
  // illegal use of LDE
  throw trap_security_exception(insn.bits());
}

// determine if it is a fast or strong memory format
if (get_field(STATE.msecregcfg->read(), MSECREGCFG_FORMAT_SEL) == FORMAT_SEL_FAST)
{
  union mojov_mem_fast_t ctval;
  union mojov_mem_fast_t ptval;

  // Mojo-V: read 3rd-party encrypted ciphertext into MEMVAL
  ctval.ct = MMU.load<uint128_t>(BASE_RS1 + insn.i_imm());

  // decrypt the value with the processor-internal key
  simon_128_128_decrypt(&p->simon_state, ctval.ct, &ptval.ct);

  if (ptval.pt.auth_sig != MOJOV_PT_SIG)
  {
    // Mojo-V not valid ciphertext, trap out...
    throw trap_security_exception(insn.bits());
  }

  // Mojo-V: all good, write the decrypted 3rd-party value to the secret register
  WRITE_RD(ptval.pt.val);
}
else if (get_field(STATE.msecregcfg->read(), MSECREGCFG_FORMAT_SEL) == FORMAT_SEL_STRONG)
{
  union mojov_mem_strong_t ctval;
  union mojov_mem_strong_t ptval;

  // Mojo-V: read 3rd-party encrypted ciphertext into MEMVAL
  ctval.ct.ct_lo = MMU.load<uint128_t>(BASE_RS1 + insn.i_imm());
  ctval.ct.ct_hi = MMU.load<uint128_t>((BASE_RS1 + insn.i_imm()) + 16);

  // decrypt the value with the processor-internal key
  simon_128_128_decrypt(&p->simon_state, ctval.ct.ct_lo, &ptval.ct.ct_lo);
  simon_128_128_decrypt(&p->simon_state, ctval.ct.ct_hi, &ptval.ct.ct_hi);
  ptval.ct.ct_hi = ptval.ct.ct_hi ^ ctval.ct.ct_lo;

  if (ptval.pt.auth_sig != MOJOV_PT_SIG)
  {
    // Mojo-V not valid ciphertext, trap out...
    throw trap_security_exception(insn.bits());
  }

  // Mojo-V: all good, write the decrypted 3rd-party value to the secret register
  WRITE_RD(ptval.pt.val);
}
else
{
  // illegal use of LDE
  throw trap_security_exception(insn.bits());
}

