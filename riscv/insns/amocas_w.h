require_extension(EXT_ZACAS);
WRITE_RD(sext32(MMU.amo_compare_and_swap<uint32_t>(BASE_RS1, RD, RS2)));
