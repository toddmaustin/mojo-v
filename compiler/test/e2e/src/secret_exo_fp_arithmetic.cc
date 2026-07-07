// End-to-end: EXO encrypted FP arithmetic on a SECRET-annotated double.
//
// SecretTaint correctly marks the double load as !secret.  SecretRegClass is
// integer-only and does NOT wrap it with mojov.secret — there is no SecretFPR
// register class.  Security is enforced by the EXO intrinsics themselves:
// _fenc loads the value into f28 and stores it via FSDE (custom opcode 0xb,
// funct3=3), and _fadd uses FLDE/FSDE around fadd.d.
//
// TAINT-LABEL: define{{.*}} @secret_exo_fp_arithmetic
// TAINT:       load double{{.*}}!secret
//
// ASM-LABEL: secret_exo_fp_arithmetic:
// ASM:        fld ft8,
// ASM:        .insn s 11, 3, ft8,
// ASM:        fadd.d
// ASM:        .insn s 11, 3,

#include "libmin.h"
#include "mojov-utils.h"
#include "dc-fast.h"
typedef mojov_mem_fast_u64_t  _uint64e_t;
typedef mojov_mem_fast_fp64_t _fp64e_t;
#include "mojov-intrinsics.h"

#define SECRET __attribute__((annotate("secret")))

extern "C" {

_fp64e_t secret_exo_fp_arithmetic(double pub_val) {
    double SECRET secret_val = 1.5;
    _fp64e_t enc_secret = _fenc(secret_val);
    _fp64e_t enc_pub    = _fenc(pub_val);
    return _fadd(enc_secret, enc_pub);
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

    // 1.5 + 2.5 = 4.0, all exact in IEEE 754
    _fp64e_t result = secret_exo_fp_arithmetic(2.5);

    simon_128_128_keyexpand(&simon_state_g, simon_key_g, 68);
    double val = mojov_decrypt_fast_fp64(&simon_state_g, result, CONTRACT_SIG);

    uint64_t bits;
    __builtin_memcpy(&bits, &val, sizeof bits);
    if (bits != (uint64_t)0x4010000000000000ULL) {
        libmin_printf("FAIL: expected 4.0 (0x4010000000000000), got 0x%lx\n", bits);
        libmin_fail(1);
    }
    libmin_printf("PASS: exo_fp_arithmetic result = 4.0\n");
    libmin_success();
    return 0;
}

}
