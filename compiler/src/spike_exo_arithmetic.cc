/*
 * spike_exo_arithmetic.cc — EXO encrypted arithmetic runs correctly on Spike.
 *
 * _enc() loads a value into X28 and stores it as SIMON ciphertext via SDE.
 * _add() decrypts both operands with LDE, adds them in-register, re-encrypts
 * with SDE.  The final ciphertext is passed back to main(), which decrypts it
 * in software (same SIMON key) and verifies the result.
 *
 * Expected: 42 (secret) + 100 (public) = 142
 *
 * .cc is required: mojov-intrinsics.h contains RISC-V inline assembly.
 */

#include <stdbool.h>
#include "libmin.h"
#include "mojov-utils.h"
#include "dc-fast.h"

typedef unsigned __int128 uint128_t;
typedef mojov_mem_fast_u64_t  _uint64e_t;
typedef mojov_mem_fast_fp64_t _fp64e_t;
#include "mojov-intrinsics.h"

#define SECRET __attribute__((annotate("secret")))

static uint128_t simon_key = SIMON128_KEY;
static simon_state_t simon_state;

static mojov_mem_fast_u64_t result_enc __attribute__((aligned(16)));

extern "C" {

static mojov_mem_fast_u64_t exo_add(uint64_t pub) {
    uint64_t SECRET secret_val = 42;
    _uint64e_t enc_secret = _enc(secret_val);
    _uint64e_t enc_pub    = _enc(pub);
    return _add(enc_secret, enc_pub);
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

    libmin_printf("Testing EXO arithmetic (42 + 100)...\n");
    result_enc = exo_add(100);

    simon_128_128_keyexpand(&simon_state, simon_key, 68);
    uint64_t val = mojov_decrypt_fast_u64(&simon_state, result_enc, CONTRACT_SIG);

    if (val != 142) {
        libmin_printf("FAIL: expected 142, got %lu\n", val);
        libmin_fail(1);
    }

    libmin_printf("PASS: EXO secret result = %lu\n", val);
    libmin_success();
    return 0;
}

} // extern "C"
