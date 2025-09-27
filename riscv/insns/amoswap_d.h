require_extension('A');
require_rv64;
WRITE_RD(MMU.amo<uint64_t>(BASE_RS1, [&](uint64_t UNUSED lhs) { return RS2; }));
