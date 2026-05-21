// End-to-end: loading from a secret-derived address leaks via cache timing.
// The compiler must refuse with a hard error.
//
// ELIM-ERROR: error: secret-dependent memory address in 'secret_mem_addr_load' leaks via cache timing

#include <stdint.h>
#define SECRET __attribute__((annotate("secret")))

// The secret index taints the GEP; the GEP is the pointer operand of the load.
uint8_t secret_mem_addr_load(uint8_t *table) {
    uint8_t SECRET idx = 7;
    return table[idx];
}
