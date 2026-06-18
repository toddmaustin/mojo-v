/*
 * spike_multi_secret.c — Two independent SECRET values in the same function.
 *
 * RA must allocate key_a and key_b to separate registers within X24-X31
 * simultaneously.  Both stores become SDE, writing to distinct 16-byte
 * ciphertext buffers.  main() decrypts both and verifies the values.
 *
 * Expected: 11 + 4 = 15  and  23 + 4 = 27
 */

#include <stdbool.h>
#include "libmin.h"
#include "mojov-utils.h"
#include "dc-fast.h"

#define SECRET __attribute__((annotate("secret")))

typedef unsigned __int128 uint128_t;

static uint128_t simon_key = SIMON128_KEY;
static simon_state_t simon_state;

static mojov_mem_fast_u64_t result_a __attribute__((aligned(16)));
static mojov_mem_fast_u64_t result_b __attribute__((aligned(16)));

static void two_secrets(uint64_t pub, uint64_t *out_a, uint64_t *out_b) {
    uint64_t SECRET key_a = 11;
    uint64_t SECRET key_b = 23;
    *out_a = key_a + pub;
    *out_b = key_b + pub;
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

    libmin_printf("Testing two independent secrets (11+4, 23+4)...\n");
    two_secrets(4, (uint64_t *)&result_a, (uint64_t *)&result_b);

    simon_128_128_keyexpand(&simon_state, simon_key, 68);

    uint64_t val_a = mojov_decrypt_fast_u64(&simon_state, result_a, CONTRACT_SIG);
    uint64_t val_b = mojov_decrypt_fast_u64(&simon_state, result_b, CONTRACT_SIG);

    if (val_a != 15) {
        libmin_printf("FAIL: expected val_a=15, got %lu\n", val_a);
        libmin_fail(1);
    }
    if (val_b != 27) {
        libmin_printf("FAIL: expected val_b=27, got %lu\n", val_b);
        libmin_fail(1);
    }

    libmin_printf("PASS: multi-secret results = %lu, %lu\n", val_a, val_b);
    libmin_success();
    return 0;
}
