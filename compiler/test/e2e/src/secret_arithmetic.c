// End-to-end: basic secret arithmetic — taint propagation and register allocation.
//
// TAINT-LABEL: define{{.*}} @compute
// TAINT:       load i8{{.*}}!secret
// TAINT:       xor i32{{.*}}!secret
//
// REGCLASS-LABEL: define{{.*}} @compute
// REGCLASS:        call i64 @llvm.riscv.mojov.secret.i64
//
// ASM-LABEL: compute:
// ASM:        {{s[bdwh]}} {{a[6-7]|s[2-9]|s10|s11|t[3-6]}},

#include <stdint.h>
#define SECRET __attribute__((annotate("secret")))

void compute(uint8_t pub, uint8_t *out) {
    uint8_t SECRET key = 0x42;
    *out = key ^ pub;
}
