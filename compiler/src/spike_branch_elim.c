/*
 * spike_branch_elim.c — SecretBranchElim correctness on Spike.
 *
 * A SECRET-annotated threshold drives a ternary expression.
 * SecretBranchElim must convert the secret-dependent branch to
 * czero.eqz/czero.nez (Zicond); no conditional branch on a secret register
 * may survive to assembly.
 *
 * Two failure modes are caught:
 * 1. Branch not eliminated: the binary contains bnez/beq on x24-x31.
 *    With secreg_mode active the hardware traps (mcause=0x1f) before
 *    libmin_success() is reached.
 * 2. Branch eliminated but czero inverts the condition: the software
 *    decryption produces 200 instead of 100 and the explicit check fails.
 *
 * Expected: threshold (42) > 20, so result must be 100.
 */

#include <stdbool.h>
#include <stdint.h>
#include "libmin.h"
#include "mojov-utils.h"
#include "dc-fast.h"

#define SECRET __attribute__((annotate("secret")))

typedef unsigned __int128 uint128_t;

static uint128_t simon_key = SIMON128_KEY;
static simon_state_t simon_state;

static mojov_mem_fast_u64_t result_enc __attribute__((aligned(16)));

static void compute(void) {
    uint64_t SECRET threshold = 42;
    /*
     * This ternary on a secret condition must be compiled to czero
     * instructions.  result is tainted (select driven by a secret icmp),
     * so the store below becomes SDE via SecretMemSubst.
     */
    uint64_t result = (threshold > 20) ? 100ULL : 200ULL;
    *(uint64_t *)&result_enc = result;
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

    libmin_printf("Testing secret-branch elimination (42 > 20 -> 100)...\n");
    compute();

    simon_128_128_keyexpand(&simon_state, simon_key, 68);
    uint64_t val = mojov_decrypt_fast_u64(&simon_state, result_enc, CONTRACT_SIG);

    if (val != 100) {
        libmin_printf("FAIL: expected 100, got %lu\n", val);
        libmin_fail(1);
    }

    libmin_printf("PASS: branch-eliminated result = %lu\n", val);
    libmin_success();
    return 0;
}
