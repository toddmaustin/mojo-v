/*
 * spike_demo.c — First Mojo-V binary run through the LLVM compiler on Spike.
 *
 * The compiler's SECRET annotation causes it to allocate `secret_b` to a
 * secret register (x24-x31) and emit SDE/LDE for any spill/reload.  The
 * result lives in the ciphertext buffer `result_enc`.  After the compute
 * function returns, main() decrypts the ciphertext in software using the
 * SIMON cipher (same key the hardware uses) to verify the computation was
 * correct end-to-end.
 *
 * Build and run:
 *   ./run_spike.sh test/spike/spike_demo.c
 */

#include <stdbool.h>
#include "libmin.h"
#include "mojov-utils.h"
#include "dc-fast.h"

#define SECRET __attribute__((annotate("secret")))

typedef unsigned __int128 uint128_t;

/* SIMON cipher state for software-side verification */
static uint128_t simon_key = SIMON128_KEY;
static simon_state_t simon_state;

/*
 * 128-bit aligned output buffer.  Our compiler emits SDE (not SD) when the
 * source register is x24-x31, writing 16 bytes of ciphertext here.
 */
static mojov_mem_fast_u64_t result_enc __attribute__((aligned(16)));

/*
 * do_secret_add — adds pub_a + 42 where 42 is SECRET.
 *
 * The compiler allocates `secret_b` to a secret register (x24-x31) and, if
 * it spills, uses SDE to write the encrypted value.  The ADD result is also
 * in a secret register; the final store `*out = ...` becomes SDE.
 */
static void
do_secret_add(uint64_t pub_a, uint64_t *out)
{
    uint64_t SECRET secret_b = 42;
    *out = pub_a + secret_b;
}

int
main(void)
{
    /* Load the fast-mode data contract (embedded hex from dc-fast.h). */
    if (mojov_configure_kmsm_from_dc_fast() != 0) {
        libmin_printf("FAIL: mojov_configure_kmsm_from_dc_fast\n");
        libmin_fail(1);
    }

    /* Enable secret register enforcement in the hardware. */
    if (mojov_enable_and_verify() != 0) {
        libmin_printf("FAIL: mojov_enable_and_verify\n");
        libmin_fail(1);
    }

    libmin_printf("Mojo-V enabled. Running secret addition (100 + 42)...\n");

    /*
     * Call the SECRET-using function.  After this returns, result_enc holds
     * 128-bit ciphertext written by the SDE instruction.
     */
    do_secret_add(100, (uint64_t *)&result_enc);

    /*
     * Software verification: initialise SIMON with the same key the hardware
     * used, then decrypt and check the signature + value.
     */
    simon_128_128_keyexpand(&simon_state, simon_key, 68);

    uint64_t val = mojov_decrypt_fast_u64(&simon_state, result_enc, CONTRACT_SIG);

    if (val != 142) {
        libmin_printf("FAIL: expected 142, got %lu\n", val);
        libmin_fail(1);
    }

    libmin_printf("PASS: secret result = %lu\n", val);
    libmin_success();
    return 0;
}
