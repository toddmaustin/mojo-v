// End-to-end: public values must not acquire !secret metadata.
//
// TAINT-LABEL: define{{.*}} @public_only
// TAINT-NOT:   !secret
// TAINT:        ret
//
// TAINT-LABEL: define{{.*}} @mixed
// TAINT:        load i8{{.*}}!secret

#include <stdint.h>
#define SECRET __attribute__((annotate("secret")))

uint8_t public_only(uint8_t a, uint8_t b) {
    return a + b;
}

void mixed(uint8_t pub, uint8_t *out) {
    uint8_t SECRET key = 0x7F;
    *out = key ^ pub;
}
