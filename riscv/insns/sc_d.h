require_extension('A');
require_rv64;

bool have_reservation = MMU.store_conditional<uint64_t>(BASE_RS1, RS2);

WRITE_RD(!have_reservation);
