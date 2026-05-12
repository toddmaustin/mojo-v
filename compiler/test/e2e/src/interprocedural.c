// End-to-end: interprocedural taint — secret returned from callee taints caller.
//
// TAINT-LABEL: define{{.*}} @get_secret
// TAINT:       load i8{{.*}}!secret
//
// TAINT-LABEL: define{{.*}} @use_secret
// TAINT:       call{{.*}}@get_secret{{.*}}!secret
//
// REGCLASS-LABEL: define{{.*}} @use_secret
// REGCLASS:        call i64 @llvm.riscv.mojov.secret.i64

#include <stdint.h>
#define SECRET __attribute__((annotate("secret")))

uint8_t get_secret(void) {
    uint8_t SECRET val = 0x99;
    return val;
}

void use_secret(uint8_t *out) {
    uint8_t s = get_secret();
    *out = s;
}
