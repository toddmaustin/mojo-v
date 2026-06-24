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

#include <stdbool.h>
#include <stdint.h>
#include "libmin.h"
#include "mojov-utils.h"
#include "dc-fast.h"

#define SECRET __attribute__((annotate("secret")))

uint8_t get_secret(void) {
    uint8_t SECRET val = 0x99;
    return val;
}

void use_secret(uint8_t *out) {
    uint8_t s = get_secret();
    *out = s;
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

    use_secret((uint8_t *)&result_enc);

    simon_128_128_keyexpand(&simon_state, simon_key, 68);
    uint64_t val = mojov_decrypt_fast_u64(&simon_state, result_enc, CONTRACT_SIG);

    if (val != 0x99) {
        libmin_printf("FAIL: expected 0x99, got 0x%lx\n", val);
        libmin_fail(1);
    }
    libmin_printf("PASS: interprocedural result = 0x%lx\n", val);
    libmin_success();
    return 0;
}
