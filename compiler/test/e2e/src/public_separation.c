// End-to-end: public values must not acquire !secret metadata.
//
// TAINT-LABEL: define{{.*}} @public_only
// TAINT-NOT:   !secret
// TAINT:        ret
//
// TAINT-LABEL: define{{.*}} @mixed
// TAINT:        load i8{{.*}}!secret

#include <stdbool.h>
#include <stdint.h>
#include "libmin.h"
#include "mojov-utils.h"
#include "dc-fast.h"

#define SECRET __attribute__((annotate("secret")))

uint8_t public_only(uint8_t a, uint8_t b) {
    return a + b;
}

void mixed(uint8_t pub, uint8_t *out) {
    uint8_t SECRET key = 0x7F;
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

    // public_only uses only public registers — result is safe to check directly
    uint8_t pub_result = public_only(3, 4);
    if (pub_result != 7) {
        libmin_printf("FAIL: public_only expected 7, got %u\n", (unsigned)pub_result);
        libmin_fail(1);
    }

    // mixed stores key^pub via SDE; verify by decryption
    mixed(0x2F, (uint8_t *)&result_enc);
    simon_128_128_keyexpand(&simon_state, simon_key, 68);
    uint64_t val = mojov_decrypt_fast_u64(&simon_state, result_enc, CONTRACT_SIG);
    if (val != (uint8_t)(0x7F ^ 0x2F)) {
        libmin_printf("FAIL: mixed expected 0x50, got 0x%lx\n", val);
        libmin_fail(1);
    }

    libmin_printf("PASS: public_separation\n");
    libmin_success();
    return 0;
}
