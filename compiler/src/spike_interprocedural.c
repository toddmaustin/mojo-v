/*
 * spike_interprocedural.c — Secret value returned from a callee is correctly
 * stored via SDE in the calling function.
 *
 * get_secret_val() holds a SECRET-annotated local and returns it.  SecretTaint
 * marks the call result in combine() as secret; SecretRegClass wraps it with
 * the mojov.secret intrinsic so RA allocates it to X24-X31.  The subsequent
 * store *out = s + pub becomes SDE.  main() decrypts and verifies the result.
 *
 * Expected: 17 (secret) + 5 (public) = 22
 */

#include <stdbool.h>
#include "libmin.h"
#include "mojov-utils.h"
#include "dc-fast.h"

#define SECRET __attribute__((annotate("secret")))

typedef unsigned __int128 uint128_t;

static uint128_t simon_key = SIMON128_KEY;
static simon_state_t simon_state;

static mojov_mem_fast_u64_t result_enc __attribute__((aligned(16)));

static uint64_t get_secret_val(void) {
    uint64_t SECRET key = 17;
    return key;
}

static void combine(uint64_t pub, uint64_t *out) {
    uint64_t s = get_secret_val();
    *out = s + pub;
}

int main(void) {
    if (mojov_configure_kmsm_from_dc_fast() != 0) {
        libmin_printf("FAIL: mojov_configure_kmsm_from_dc_fast\n");
        libmin_fail(1);
    }
    if (mojov_enable_and_verify() != 0) {
        libmin_printf("FAIL: mojov_enable_and_verify\n");
        libmin_fail(1);
    }

    libmin_printf("Testing interprocedural secret (17 + 5)...\n");
    combine(5, (uint64_t *)&result_enc);

    simon_128_128_keyexpand(&simon_state, simon_key, 68);
    uint64_t val = mojov_decrypt_fast_u64(&simon_state, result_enc, CONTRACT_SIG);

    if (val != 22) {
        libmin_printf("FAIL: expected 22, got %lu\n", val);
        libmin_fail(1);
    }

    libmin_printf("PASS: interprocedural secret result = %lu\n", val);
    libmin_success();
    return 0;
}
