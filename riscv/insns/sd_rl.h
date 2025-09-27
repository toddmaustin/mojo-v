require_rv64;
require_extension(EXT_ZALASR);
MMU.store<uint64_t>(BASE_RS1, RS2);
