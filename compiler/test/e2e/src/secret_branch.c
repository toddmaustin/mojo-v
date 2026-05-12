// End-to-end: branching on secret must be converted to select.
// The C pattern below produces a diamond that SecretBranchElim can handle.
//
// TAINT-LABEL: define{{.*}} @classify
// TAINT:       icmp{{.*}}!secret
//
// ELIM-LABEL: define{{.*}} @classify
// ELIM:        select i1
// ELIM-NOT:    br i1

#include <stdint.h>
#define SECRET __attribute__((annotate("secret")))

// Assign one of two public values based on a secret threshold.
// Both arms store into a local and then fall through to a single return,
// producing the diamond shape SecretBranchElim requires.
uint8_t classify(uint8_t pub_a, uint8_t pub_b) {
    uint8_t SECRET threshold = 100;
    uint8_t result;
    if (threshold > 50) {
        result = pub_a;
    } else {
        result = pub_b;
    }
    return result;
}
