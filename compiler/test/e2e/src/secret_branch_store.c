// End-to-end: secret-dependent branch with stores in both arms cannot be
// converted to select — executing the wrong store would corrupt memory.
// The compiler must refuse with a hard error.
//
// ELIM-ERROR: error: secret-dependent branch in 'secret_branch_store' cannot be converted to select

#include <stdint.h>
#define SECRET __attribute__((annotate("secret")))

void secret_branch_store(uint8_t *a, uint8_t *b) {
    uint8_t SECRET threshold = 100;
    if (threshold > 50) {
        *a = 1;
    } else {
        *b = 2;
    }
}
