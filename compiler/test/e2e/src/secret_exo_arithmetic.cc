// End-to-end: EXO encrypted arithmetic on a secret-annotated value.
//
// A SECRET-annotated uint64_t is encrypted with _enc() and added to a public
// encrypted value with _add().  Verifies that:
//   - libmin (for I/O) and exo (for secret operations) are correctly separated
//   - SecretTaint seeds taint from the SECRET annotation and propagates it
//     through the EXO inline-asm intrinsics (_enc, _add)
//   - SecretRegClass wraps the tainted integer load with the secret intrinsic
//
// TAINT-LABEL: define{{.*}} @secret_exo_arithmetic
// TAINT:       load i64{{.*}}!secret
//
// Note: no REGCLASS check here.  The secret_val alloca is promoted by mem2reg
// after SecretTaint strips the llvm.var.annotation call; the constant 42 then
// flows directly into EXO's inline asm, which internally moves it to x28 (a
// SecretGPR) without an explicit @llvm.riscv.mojov.secret.i64 wrapper.

#include "libmin.h"
#include "mojov-utils.h"
#include "dc-fast.h"
typedef mojov_mem_fast_u64_t  _uint64e_t;
typedef mojov_mem_fast_fp64_t _fp64e_t;
#include "mojov-intrinsics.h"

#define SECRET __attribute__((annotate("secret")))

// Encrypt a secret value and add a (separately encrypted) public value to it.
// The result is an encrypted struct; secrets never leave the secret register
// domain during the computation.
//
// extern "C" prevents C++ name mangling so the IR function name matches the
// FileCheck LABEL patterns above (e.g. @secret_exo_arithmetic, not
// @_Z21secret_exo_arithmeticm).
extern "C" {
_uint64e_t secret_exo_arithmetic(uint64_t pub_val) {
    uint64_t SECRET secret_val = 42;
    _uint64e_t enc_secret = _enc(secret_val);
    _uint64e_t enc_pub    = _enc(pub_val);
    return _add(enc_secret, enc_pub);
}

static uint128_t simon_key_g = SIMON128_KEY;
static simon_state_t simon_state_g;

int main(void) {
    if (mojov_configure_kmsm_from_dc_fast() != 0) {
        libmin_printf("FAIL: KMSM setup failed\n");
        libmin_fail(1);
    }
    if (mojov_enable_and_verify() != 0) {
        libmin_printf("FAIL: secreg enable failed\n");
        libmin_fail(1);
    }

    _uint64e_t result = secret_exo_arithmetic(100);

    simon_128_128_keyexpand(&simon_state_g, simon_key_g, 68);
    uint64_t val = mojov_decrypt_fast_u64(&simon_state_g, result, CONTRACT_SIG);

    if (val != 142) {
        libmin_printf("FAIL: expected 142, got %lu\n", val);
        libmin_fail(1);
    }
    libmin_printf("PASS: exo_arithmetic result = %lu\n", val);
    libmin_success();
    return 0;
}
}
