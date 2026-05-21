// End-to-end: storing to a secret-derived address leaks via cache timing,
// even when the value being stored is public. The compiler must refuse.
//
// ELIM-ERROR: error: secret-dependent memory address in 'secret_mem_addr_store' leaks via cache timing

#include <stdint.h>
#define SECRET __attribute__((annotate("secret")))

// val is public; only the address is secret. Still a cache-timing leak.
void secret_mem_addr_store(uint8_t *table, uint8_t val) {
    uint8_t SECRET idx = 3;
    table[idx] = val;
}
