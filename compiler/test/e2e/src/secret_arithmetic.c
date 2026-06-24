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
// ASM:        sde {{s8|s9|s10|s11|t3|t4|t5|t6}},

#include <stdbool.h>
#include <stdint.h>
#include "libmin.h"
#include "mojov-utils.h"
#include "dc-fast.h"

#define SECRET __attribute__((annotate("secret")))

void compute(uint8_t pub, uint8_t *out) {
    uint8_t SECRET key = 0x42;
    *out = key ^ pub;
}

static uint128_t simon_key = SIMON128_KEY;
static simon_state_t simon_state;
static mojov_mem_fast_u64_t result_enc __attribute__((aligned(16)));

int main(void) {
    if (mojov_configure_kmsm_from_dc_fast() != 0) {
        libmin_printf("FAIL: KMSM setup failed\n");
        libmin_fail(1);
    }
    if (mojov_enable_and_verify() != 0) {
        libmin_printf("FAIL: secreg enable failed\n");
        libmin_fail(1);
    }

    compute(0x11, (uint8_t *)&result_enc);

    simon_128_128_keyexpand(&simon_state, simon_key, 68);
    uint64_t val = mojov_decrypt_fast_u64(&simon_state, result_enc, CONTRACT_SIG);

    if (val != (uint8_t)(0x42 ^ 0x11)) {
        libmin_printf("FAIL: expected 0x53, got 0x%lx\n", val);
        libmin_fail(1);
    }
    libmin_printf("PASS: secret_arithmetic result = 0x%lx\n", val);
    libmin_success();
    return 0;
}
