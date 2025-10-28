require_extension(EXT_ZKMOJOV);

// Mojo-V: LDE requires SECREGs enabled AND RD must be secret reg
if (!p->get_secreg_mode() || !IS_SECREG(insn.rd()))
{
  // illegal use of LDE
  throw trap_security_exception(insn.bits());
}

union mojov_memfmt_t ctval;
union mojov_memfmt_t ptval;

// Mojo-V: read 3rd-party encrypted ciphertext into MEMVAL
ctval.ct = MMU.load<uint128_t>(BASE_RS1 + insn.i_imm());

// decrypt the value with the processor-internal key
simon_128_128_decrypt(&p->simon_state, ctval.ct, &ptval.ct);

if (ptval.pt.sig != MOJOV_PT_SIG)
{
  // Mojo-V not valid ciphertext, trap out...
  throw trap_security_exception(insn.bits());
}

// Mojo-V: all good, write the decrypted 3rd-party value to the secret register
WRITE_RD(ptval.pt.val);

