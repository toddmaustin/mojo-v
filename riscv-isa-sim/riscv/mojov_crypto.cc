#include "mojov_crypto.h"

#include <cassert>
#include <cstring>

#include <openssl/core_names.h>
#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <openssl/pem.h>
#include <openssl/provider.h>

EVP_PKEY *mojov_read_privkey_pem(const char *path) {
  BIO *bio = BIO_new_file(path, "rb");
  if (!bio) return NULL;
  EVP_PKEY *pkey = PEM_read_bio_PrivateKey(bio, NULL, NULL, NULL);
  BIO_free(bio);
  return pkey;
}

int mojov_hkdf_sha256_key128(const unsigned char *ss, size_t ss_len,
                             unsigned char key_out[16]) {
  int ok = 0;
  EVP_KDF *kdf = NULL;
  EVP_KDF_CTX *kctx = NULL;

  static const unsigned char salt[] = "dc-multitool-salt";
  static const unsigned char info[] = "mlkem512-simon128-data_contract";

  char digest_name[] = "SHA256";
  OSSL_PARAM params[] = {
      OSSL_PARAM_construct_utf8_string(OSSL_KDF_PARAM_DIGEST, digest_name, 0),
      OSSL_PARAM_construct_octet_string(OSSL_KDF_PARAM_KEY, (void *)ss, ss_len),
      OSSL_PARAM_construct_octet_string(OSSL_KDF_PARAM_SALT, (void *)salt, sizeof(salt) - 1),
      OSSL_PARAM_construct_octet_string(OSSL_KDF_PARAM_INFO, (void *)info, sizeof(info) - 1),
      OSSL_PARAM_construct_end()
  };

  kdf = EVP_KDF_fetch(NULL, "HKDF", NULL);
  if (!kdf) goto end_exit;
  kctx = EVP_KDF_CTX_new(kdf);
  if (!kctx) goto end_exit;

  ok = EVP_KDF_derive(kctx, key_out, 16, params);

end_exit:
  EVP_KDF_CTX_free(kctx);
  EVP_KDF_free(kdf);
  return ok;
}

int mojov_sha256_bytes(const unsigned char *in, size_t inlen, unsigned char out32[32]) {
  int ok = 0;
  EVP_MD *md = NULL;
  EVP_MD_CTX *mctx = NULL;
  unsigned int outlen = 0;

  md = EVP_MD_fetch(NULL, "SHA256", NULL);
  if (!md) goto end;
  mctx = EVP_MD_CTX_new();
  if (!mctx) goto end;
  if (EVP_DigestInit_ex(mctx, md, NULL) != 1) goto end;
  if (EVP_DigestUpdate(mctx, in, inlen) != 1) goto end;
  if (EVP_DigestFinal_ex(mctx, out32, &outlen) != 1) goto end;
  if (outlen != 32) goto end;

  ok = 1;
end:
  EVP_MD_CTX_free(mctx);
  EVP_MD_free(md);
  return ok;
}

void mojov_simon_decrypt(simon_state_t *simon_state,
                         const unsigned char *ct, size_t ct_len,
                         unsigned char *pt)
{
  assert(ct_len % (128/8) == 0);

  uint128_t *psrc = (uint128_t *)ct, *pdst = (uint128_t *)pt;

  uint128_t iv = 0;
  for (unsigned i=0; i < ct_len/(128/8); i++)
  {
    simon_128_128_decrypt(simon_state, psrc[i], &pdst[i]);
    pdst[i] = pdst[i] ^ iv;
    iv = psrc[i];
  }
}

uint64_t mojov_load_u64_be(const unsigned char *src) {
  return ((uint64_t)src[0] << 56) |
         ((uint64_t)src[1] << 48) |
         ((uint64_t)src[2] << 40) |
         ((uint64_t)src[3] << 32) |
         ((uint64_t)src[4] << 24) |
         ((uint64_t)src[5] << 16) |
         ((uint64_t)src[6] <<  8) |
         ((uint64_t)src[7] <<  0);
}

void mojov_dc_decode(data_contract_t *dc, const unsigned char in[64]) {
  dc->salt         = mojov_load_u64_be(in + 0);
  memcpy(dc->sig,         in + 8,  16);
  memcpy(dc->sym_key_128, in + 24, 16);
  dc->contract_sig = mojov_load_u64_be(in + 40);
  dc->ciphers      = mojov_load_u64_be(in + 48);
  dc->format_sel   = in[56];
  memcpy(dc->pad, in + 57, 7);
}

bool mojov_open_contract_from_components(const char *sk_pem_path,
                                         const unsigned char *kem_dc,
                                         size_t kem_dc_len,
                                         const unsigned char *msg_dc,
                                         size_t msg_dc_len,
                                         const unsigned char *expected_pt_sha_or_null,
                                         data_contract_t *out_dc,
                                         unsigned char out_pt_sha[32],
                                         mojov_open_status_t *status)
{
  constexpr size_t DC_WIRE_LEN = 64;
  constexpr size_t SIMON128_KEY_LEN = 16;

  if (status) *status = mojov_open_status_t::OK;

  if (!sk_pem_path || !*sk_pem_path || !kem_dc || kem_dc_len == 0 || !msg_dc || msg_dc_len != DC_WIRE_LEN || !out_dc || !out_pt_sha) {
    if (status) *status = (!sk_pem_path || !*sk_pem_path) ? mojov_open_status_t::MISSING_SK_PATH : mojov_open_status_t::BAD_INPUT;
    return false;
  }

  OSSL_PROVIDER *prov = OSSL_PROVIDER_load(NULL, "default");
  if (!prov) {
    if (status) *status = mojov_open_status_t::OSSL_PROVIDER_LOAD;
    return false;
  }

  bool ok = false;
  EVP_PKEY *sk = nullptr;
  EVP_PKEY_CTX *dctx = nullptr;
  unsigned char *ss = nullptr;

  sk = mojov_read_privkey_pem(sk_pem_path);
  if (!sk) { if (status) *status = mojov_open_status_t::SK_LOAD; goto out; }

  dctx = EVP_PKEY_CTX_new_from_pkey(NULL, sk, NULL);
  if (!dctx) { if (status) *status = mojov_open_status_t::DECAP_CTX; goto out; }
  if (EVP_PKEY_decapsulate_init(dctx, NULL) != 1) { if (status) *status = mojov_open_status_t::DECAP_INIT; goto out; }

  size_t ss_len = 0;
  if (EVP_PKEY_decapsulate(dctx, NULL, &ss_len, kem_dc, kem_dc_len) != 1) {
    if (status) *status = mojov_open_status_t::DECAP_SIZE; goto out;
  }

  ss = (unsigned char *)OPENSSL_malloc(ss_len);
  if (!ss) { if (status) *status = mojov_open_status_t::SS_MALLOC; goto out; }

  if (EVP_PKEY_decapsulate(dctx, ss, &ss_len, kem_dc, kem_dc_len) != 1) {
    if (status) *status = mojov_open_status_t::DECAP; goto out;
  }

  unsigned char k[SIMON128_KEY_LEN];
  if (mojov_hkdf_sha256_key128(ss, ss_len, k) != 1) {
    if (status) *status = mojov_open_status_t::HKDF; goto out;
  }

  simon_state_t local_simon_state;
  simon_128_128_keyexpand(&local_simon_state, *((uint128_t *)k), 68);

  unsigned char pt[DC_WIRE_LEN];
  mojov_simon_decrypt(&local_simon_state, msg_dc, msg_dc_len, pt);

  if (mojov_sha256_bytes(pt, sizeof(pt), out_pt_sha) != 1) {
    if (status) *status = mojov_open_status_t::SHA256; goto out;
  }

  if (expected_pt_sha_or_null && CRYPTO_memcmp(out_pt_sha, expected_pt_sha_or_null, 32) != 0) {
    if (status) *status = mojov_open_status_t::PT_HASH_MISMATCH; goto out;
  }

  mojov_dc_decode(out_dc, pt);

  static const char sig_str[16+1] = "Mojo-V ver. #001";
  if (CRYPTO_memcmp(out_dc->sig, sig_str, 16) != 0) {
    if (status) *status = mojov_open_status_t::BAD_SIGNATURE; goto out;
  }

  OPENSSL_cleanse(k, sizeof(k));
  OPENSSL_cleanse(ss, ss_len);

  ok = true;
out:
  if (ss) OPENSSL_free(ss);
  EVP_PKEY_CTX_free(dctx);
  EVP_PKEY_free(sk);
  OSSL_PROVIDER_unload(prov);
  return ok;
}
