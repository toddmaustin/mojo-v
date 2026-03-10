#ifndef _RISCV_MOJOV_CRYPTO_H
#define _RISCV_MOJOV_CRYPTO_H

#include <cstddef>
#include <cstdint>
#include <openssl/evp.h>
#include "decode.h"
#include "simon.h"

enum class mojov_open_status_t : int {
  OK = 0,
  MISSING_SK_PATH = 1,
  OSSL_PROVIDER_LOAD = 2,
  SK_LOAD = 4,
  DECAP_CTX = 5,
  DECAP_INIT = 6,
  DECAP_SIZE = 7,
  SS_MALLOC = 8,
  DECAP = 9,
  HKDF = 10,
  BAD_SIGNATURE = 11,
  BAD_INPUT = 12,
  SHA256 = 13,
  PT_HASH_MISMATCH = 14,
};

EVP_PKEY *mojov_read_privkey_pem(const char *path);
int mojov_hkdf_sha256_key128(const unsigned char *ss, size_t ss_len,
                             unsigned char key_out[16]);
int mojov_sha256_bytes(const unsigned char *in, size_t inlen, unsigned char out32[32]);
void mojov_simon_decrypt(simon_state_t *simon_state,
                         const unsigned char *ct, size_t ct_len,
                         unsigned char *pt);
uint64_t mojov_load_u64_be(const unsigned char *src);
void mojov_dc_decode(data_contract_t *dc, const unsigned char in[64]);

bool mojov_open_contract_from_components(const char *sk_pem_path,
                                         const unsigned char *kem_dc,
                                         size_t kem_dc_len,
                                         const unsigned char *msg_dc,
                                         size_t msg_dc_len,
                                         const unsigned char *expected_pt_sha_or_null,
                                         data_contract_t *out_dc,
                                         unsigned char out_pt_sha[32],
                                         mojov_open_status_t *status);

#endif
