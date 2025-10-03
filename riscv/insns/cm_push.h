require_zcmp_pushpop;

// Mojo-V: this instruction is disallowed when SECRET registers are enabled
if (p->extension_enabled(EXT_ZKMOJOV) && p->get_secreg_mode())
{
  // illegal use of SDE
  throw trap_illegal_instruction(insn.bits());
}

const auto new_sp = SP - insn.zcmp_stack_adjustment(xlen);
auto addr = BASE_SP;

for (int i = Sn(11); i >= 0; i--) {
  if (insn.zcmp_regmask() & (1 << i)) {
    addr -= xlen / 8;

    if (xlen == 32)
      MMU.store<uint32_t>(addr, READ_REG(i));
    else
      MMU.store<uint64_t>(addr, READ_REG(i));
  }
}

WRITE_REG(X_SP, new_sp);
