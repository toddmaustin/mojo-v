require_extension(EXT_ZCA);
if (BRPRED_RVC_RS1S == 0)
  set_pc(pc + insn.rvc_b_imm());
