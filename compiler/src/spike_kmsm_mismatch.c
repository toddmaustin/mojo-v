/*
 * spike_kmsm_mismatch.c — KMSM contract validation rejects a wrong secret key.
 *
 * The data contract in dc-fast.h was encapsulated with the project's public
 * key (bringup-bench/target/pk-file.pem).  This test is run with a different
 * secret key (bringup-bench/target/wrong-sk.pem) so that decapsulation yields
 * a wrong shared secret, making the HMAC / signature check inside
 * mojov_configure_kmsm_from_dc_fast() fail.
 *
 * Run with:
 *   MOJOV_SK=bringup-bench/target/wrong-sk.pem ./run_spike.sh spike_kmsm_mismatch.c
 *
 * Expected: mojov_configure_kmsm_from_dc_fast() returns non-zero → PASS.
 * If a mismatched key is somehow accepted the test explicitly fails.
 */

#include <stdbool.h>
#include <stdint.h>
#include "libmin.h"
#include "mojov-utils.h"
#include "dc-fast.h"

int main(void) {
    libmin_printf("Testing KMSM rejection of wrong secret key...\n");

    int rc = mojov_configure_kmsm_from_dc_fast();
    if (rc != 0) {
        libmin_printf("PASS: KMSM correctly rejected mismatched key (rc=%d)\n", rc);
        libmin_success();
    }

    libmin_printf("FAIL: KMSM accepted a key it should have rejected\n");
    libmin_fail(1);
    return 0;
}
