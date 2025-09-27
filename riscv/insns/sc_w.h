require_extension('A');

bool have_reservation = MMU.store_conditional<uint32_t>(BASE_RS1, RS2);

WRITE_RD(!have_reservation);
