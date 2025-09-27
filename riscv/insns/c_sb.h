require_extension(EXT_ZCB);
MMU.store<uint8_t>(BASE_RVC_RS1S + insn.rvc_lbimm(), RVC_RS2S);
