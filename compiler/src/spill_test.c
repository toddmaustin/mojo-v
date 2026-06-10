#include "libmin.h"

#define SECRET __attribute__((annotate("secret")))

// Forces a SecretGPR spill: 9 live secret values simultaneously exceeds
// the 8 available secret registers (x24-x31), so the register allocator
// must spill at least one via SDE and reload it via LDE.
uint64_t spill_test(void) {
    uint64_t SECRET a = 1, b = 2, c = 3, d = 4;
    uint64_t SECRET e = 5, f = 6, g = 7, h = 8, i = 9;

    // Use all 9 simultaneously so none can be folded away.
    uint64_t r = a + b + c + d + e + f + g + h + i;
    return r;
}

int main(void) {
    libmin_printf("spill: %llu\n", spill_test());
    libtarg_success();
}
