#ifndef MOJOV_UTILS_H
#define MOJOV_UTILS_H

#include <stdint.h>

void mojov_print_mprivregcfg(uint64_t val);
uint64_t mojov_read_mprivregcfg(void);
void mojov_write_mprivregcfg(uint64_t value);

int mojov_configure_kmsm_from_dc_fast(void);
int mojov_configure_kmsm_from_dc_strong(void);
int mojov_configure_kmsm_from_dc_proof(void);

#endif
