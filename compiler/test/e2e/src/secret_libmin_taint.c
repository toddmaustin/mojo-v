// End-to-end: taint propagates through a libmin function call.
// A secret int passed to libmin_abs must come back tainted at the call site.
// The internal branch in libmin_abs (i < 0 ? -i : i) is a pure diamond with
// no stores or calls in either arm, so SecretBranchElim converts it to a
// select and the full pipeline succeeds.
//
// TAINT-LABEL: define{{.*}} @secret_libmin_taint
// TAINT:       call{{.*}}@libmin_abs{{.*}}!secret

#include <stdint.h>
#include "libmin.h"
#define SECRET __attribute__((annotate("secret")))

int secret_libmin_taint(void) {
    int SECRET secret_val = -7;
    return libmin_abs(secret_val);
}
