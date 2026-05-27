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
// REGCLASS-LABEL: define{{.*}} @secret_exo_arithmetic
// REGCLASS:        call i64 @llvm.riscv.mojov.secret.i64

#include "libmin.h"
#include "mojov-utils.h"
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
}
