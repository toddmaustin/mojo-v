require_extension(EXT_ZICFISS);
require_extension('A');

DECLARE_XENVCFG_VARS(SSE);
require_envcfg(SSE);
WRITE_RD(sext32(MMU.ssamoswap<uint32_t>(BASE_RS1, RS2)));

