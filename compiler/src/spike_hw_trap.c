/*
 * spike_hw_trap.c — Hardware enforcement negative test.
 *
 * Verifies that the Mojo-V hardware (Spike) correctly traps when SD is
 * executed with rs2 in X24-X31 after secreg_mode is enabled.
 *
 * __main() in libtarg.c (TARGET_SPIKE) installs a default trap handler that
 * calls libmin_fail.  This test replaces mtvec with its own handler that
 * treats any trap as the expected security exception (PASS), then executes
 * "sd x28, 0(buf)" via inline assembly — intentionally bypassing the
 * compiler's SD→SDE substitution.
 *
 * If the hardware traps:   our handler runs → PASS
 * If the hardware does not trap: execution falls through → FAIL
 */

#include <stdbool.h>
#include "libmin.h"
#include "mojov-utils.h"
#include "dc-fast.h"

/* Valid mapped target for the intentional bad SD. */
static uint64_t dummy_buf[2] __attribute__((aligned(16)));

/*
 * Custom trap handler.  Aligned to 4 bytes so that mtvec bits[1:0] == 0
 * (Direct mode: all traps jump to this address).
 */
__attribute__((aligned(4), noinline))
static void security_trap_handler(void) {
    libmin_printf("PASS: hardware correctly trapped SD with secret register\n");
    libmin_success();
    while (1); /* unreachable; libmin_success() does not return */
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

    /*
     * Replace the default trap handler (libmin_fail) with one that
     * treats the upcoming security exception as the expected outcome.
     * Must be set AFTER secreg_mode is enabled so KMSM setup traps
     * (if any) are still handled by the default fail handler.
     */
    __asm__ volatile("csrw mtvec, %0" :: "r"(security_trap_handler));

    libmin_printf("Testing: SD with X28 after secreg_mode must trap...\n");

    /*
     * Load a value into x28 and store it with SD, bypassing the
     * compiler's SDE substitution.  SECREG_REF(x28) is true because
     * secreg_mode is now active and x28 is in IS_SECREG (X24-X31).
     * The hardware must raise a security exception before the store
     * completes.
     */
    __asm__ volatile(
        "li   x28, 0xdeadbeef\n\t"
        "sd   x28, 0(%0)"
        :: "r"(dummy_buf)
        : "x28", "memory"
    );

    /* If execution reaches here the hardware did not trap — test fails. */
    libmin_printf("FAIL: SD with x28 completed without trapping\n");
    libmin_fail(1);
    return 0;
}
