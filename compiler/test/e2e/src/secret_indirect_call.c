// End-to-end: calling through a function pointer derived from a secret value
// leaks the secret via branch target prediction. The compiler must refuse.
//
// Using a secret offset added to a base function pointer avoids conflating
// this with the secret-memory-address check (no secret-indexed array access).
//
// ELIM-ERROR: error: secret-dependent indirect control flow in 'secret_indirect_call' is not transformable

#include <stdint.h>
#define SECRET __attribute__((annotate("secret")))

typedef void (*handler_t)(void);

// The secret offset taints the pointer arithmetic; the resulting function
// pointer is secret; the indirect call through it is forbidden.
void secret_indirect_call(handler_t base) {
    uintptr_t SECRET offset = 0;
    handler_t fn = (handler_t)((uintptr_t)base + offset);
    fn();
}
