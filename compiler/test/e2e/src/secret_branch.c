// End-to-end: branching on secret must be converted to select.
// The C pattern below produces a diamond that SecretBranchElim can handle.
//
// TAINT-LABEL: define{{.*}} @classify
// TAINT:       icmp{{.*}}!secret
//
// ELIM-LABEL: define{{.*}} @classify
// ELIM:        select i1
// ELIM-NOT:    br i1
// ELIM:        ret
//
// ASM-LABEL: classify:
// ASM:        czero.{{nez|eqz}}
// ASM-NOT:    beq
// ASM-NOT:    bne
// ASM:        ret

#include <stdbool.h>
#include <stdint.h>
#include "libmin.h"
#include "mojov-utils.h"
#include "dc-fast.h"

#define SECRET __attribute__((annotate("secret")))

// Assign one of two public values based on a secret threshold.
// Result goes through *out (via SDE) so it never crosses a secret-to-public
// register boundary, which would trap in secreg mode.
void classify(uint8_t pub_a, uint8_t pub_b, uint8_t *out) {
    uint8_t SECRET threshold = 100;
    uint8_t result;
    if (threshold > 50) {
        result = pub_a;
    } else {
        result = pub_b;
    }
    *out = result;
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

    // threshold=100 > 50, so result = pub_a = 111
    classify(111, 222, (uint8_t *)&result_enc);

    simon_128_128_keyexpand(&simon_state, simon_key, 68);
    uint64_t val = mojov_decrypt_fast_u64(&simon_state, result_enc, CONTRACT_SIG);

    if (val != 111) {
        libmin_printf("FAIL: expected 111, got %lu\n", val);
        libmin_fail(1);
    }
    libmin_printf("PASS: branch-eliminated result = %lu\n", val);
    libmin_success();
    return 0;
}
