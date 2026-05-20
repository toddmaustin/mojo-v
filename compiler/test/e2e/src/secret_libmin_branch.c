// End-to-end: secret value passed as count to libmin_memcpy triggers
// SecretBranchElim failure.  The loop "for (; n; n--)" branches on the secret
// count and the loop body contains stores — the compiler must refuse.
//
// ELIM-ERROR: error: secret-dependent branch in 'libmin_memcpy' cannot be converted to select

#include <stdint.h>
#include "libmin.h"
#define SECRET __attribute__((annotate("secret")))

void secret_libmin_branch(char *dest, const char *src) {
    uint8_t SECRET secret_count = 4;
    libmin_memcpy(dest, src, secret_count);
}
