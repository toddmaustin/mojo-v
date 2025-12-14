// mlkem_files_demo.c
//
// Build:
//   cc -Wall -Wextra -O2 mlkem_files_demo.c -o mlkem_demo -lcrypto
//
// Usage:
//   ./dc-multitool keygen
//   ./dc-multitool dcgen {fast,strong,proof-carrying}
//   ./dc-multitool dcchk
//   ./dc-multitool all     # = keygen + dcgen (fast) + dcchk
//
// Files:
//   mlkem512_pk.pem   - ML-KEM-512 public key (PEM, text)
//   mlkem512_sk.pem   - ML-KEM-512 private key (PEM, text; unencrypted, demo only)
//   mlkem512-ct.txt   - KEM ciphertext + AES-GCM-encrypted Mojo-V data contract

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/kdf.h>
#include <openssl/provider.h>
#include <openssl/err.h>
#include <openssl/crypto.h>
#include <openssl/pem.h>
#include <openssl/core_names.h>

#define PK_FILE  "mlkem512-pk.pem"
#define SK_FILE  "mlkem512-sk.pem"
#define CT_FILE  "mlkem512-dc.txt"

#define GCM_IV_LEN     12
#define GCM_TAG_LEN    16
#define AES128_KEY_LEN 16

/* === Mojo-V data contract (512 bits) ===
 * See Mojo-V spec data_contract_t definition (512 bits, 128-bit alignment).
 *
 * struct { // alignment of 128-bits
 *   uint8_t sig[16]; // "Mojo-V ver. #001"
 *   uint128_t sym_key_128;
 *   uint64_t contract_sig;
 *   uint64_t salt;
 *   uint64_t ciphers;
 *   uint8_t format_sel; // 0=fast, 1=strong, 2=proofcarrying
 *   uint8_t __pad[];    // random padding to make total size 512 bits
 * } data_contract_t;
 */

#define DC_WIRE_LEN 64

typedef struct {
    uint64_t salt;           // random 64-bit
    uint8_t  sig[16];        // "Mojo-V ver. #001"
    uint8_t  sym_key_128[16];// 128-bit symmetric key (bytes, not C uint128_t)
    uint64_t contract_sig;   // unique 64-bit authentication signature
    uint64_t ciphers;        // 64-bit mask (0 for now)
    uint8_t  format_sel;     // 0=fast, 1=strong, 2=proofcarrying
    uint8_t  pad[7];         // random padding to reach 64 bytes
} data_contract_t;

/* ---- Utility / error ---- */
static void die_openssl(const char *what) {
    fprintf(stderr, "ERROR: %s\n", what);
    ERR_print_errors_fp(stderr);
    exit(1);
}
static void die_msg(const char *what) {
    fprintf(stderr, "ERROR: %s\n", what);
    exit(1);
}

/* ---- Endian helpers ---- */
static void store_u64_be(unsigned char *dst, uint64_t v) {
    dst[0] = (unsigned char)((v >> 56) & 0xff);
    dst[1] = (unsigned char)((v >> 48) & 0xff);
    dst[2] = (unsigned char)((v >> 40) & 0xff);
    dst[3] = (unsigned char)((v >> 32) & 0xff);
    dst[4] = (unsigned char)((v >> 24) & 0xff);
    dst[5] = (unsigned char)((v >> 16) & 0xff);
    dst[6] = (unsigned char)((v >>  8) & 0xff);
    dst[7] = (unsigned char)((v >>  0) & 0xff);
}
static uint64_t load_u64_be(const unsigned char *src) {
    return ((uint64_t)src[0] << 56) |
           ((uint64_t)src[1] << 48) |
           ((uint64_t)src[2] << 40) |
           ((uint64_t)src[3] << 32) |
           ((uint64_t)src[4] << 24) |
           ((uint64_t)src[5] << 16) |
           ((uint64_t)src[6] <<  8) |
           ((uint64_t)src[7] <<  0);
}

/* ---- Mojo-V data_contract_t init / encode / decode ---- */

static void dc_init(data_contract_t *dc, uint8_t format_sel) {
    static const char sig_str[16] = "Mojo-V ver. #001";

    memset(dc, 0, sizeof(*dc));
    memcpy(dc->sig, sig_str, 16);

    /* sym_key_128: fresh 128-bit symmetric key chosen by data owner */
    if (RAND_bytes(dc->sym_key_128, sizeof(dc->sym_key_128)) != 1)
        die_openssl("dc_init: RAND_bytes(sym_key_128)");

    /* contract_sig and salt: random 64-bit values */
    unsigned char tmp[8];

    if (RAND_bytes(tmp, sizeof(tmp)) != 1)
        die_openssl("dc_init: RAND_bytes(contract_sig)");
    dc->contract_sig = load_u64_be(tmp);

    if (RAND_bytes(tmp, sizeof(tmp)) != 1)
        die_openssl("dc_init: RAND_bytes(salt)");
    dc->salt = load_u64_be(tmp);

    dc->ciphers = 0; /* For now, per instruction */

    dc->format_sel = format_sel;

    if (RAND_bytes(dc->pad, sizeof(dc->pad)) != 1)
        die_openssl("dc_init: RAND_bytes(pad)");
}

static void dc_encode(unsigned char out[DC_WIRE_LEN], const data_contract_t *dc) {
    store_u64_be(out + 0, dc->salt);
    memcpy(out + 8,  dc->sig,          16);
    memcpy(out + 24, dc->sym_key_128,  16);
    store_u64_be(out + 40, dc->contract_sig);
    store_u64_be(out + 48, dc->ciphers);
    out[56] = dc->format_sel;
    memcpy(out + 57, dc->pad, 7);
}

static void dc_decode(data_contract_t *dc, const unsigned char in[DC_WIRE_LEN]) {
    dc->salt        = load_u64_be(in + 0);
    memcpy(dc->sig,         in + 8,  16);
    memcpy(dc->sym_key_128, in + 24, 16);
    dc->contract_sig = load_u64_be(in + 40);
    dc->ciphers     = load_u64_be(in + 48);
    dc->format_sel  = in[56];
    memcpy(dc->pad, in + 57, 7);
}

/* ---- Hex helpers ---- */
static int hex_to_nybble(char c) {
    if ('0' <= c && c <= '9') return c - '0';
    if ('a' <= c && c <= 'f') return 10 + (c - 'a');
    if ('A' <= c && c <= 'F') return 10 + (c - 'A');
    return -1;
}
static char nybble_to_hex(unsigned v) {
    return (v < 10) ? (char)('0' + v) : (char)('a' + (v - 10));
}
static char *bytes_to_hex(const unsigned char *in, size_t len) {
    char *s = (char *)OPENSSL_malloc(len * 2 + 1);
    if (!s) return NULL;
    for (size_t i = 0; i < len; i++) {
        s[2*i+0] = nybble_to_hex((in[i] >> 4) & 0xF);
        s[2*i+1] = nybble_to_hex((in[i] >> 0) & 0xF);
    }
    s[len * 2] = '\0';
    return s;
}

/* FIXED: only parse up to end-of-line, not entire file */
static int hex_to_bytes(const char *hex, unsigned char **out, size_t *outlen) {
    /* Only parse up to end-of-line (or NUL) */
    size_t n = 0;
    while (hex[n] != '\0' && hex[n] != '\n' && hex[n] != '\r')
        n++;

    /* Trim trailing spaces/tabs */
    while (n > 0 && (hex[n-1] == ' ' || hex[n-1] == '\t'))
        n--;

    if (n == 0) return 0;
    if (n % 2 != 0) return 0;

    size_t blen = n / 2;
    unsigned char *buf = (unsigned char *)OPENSSL_malloc(blen);
    if (!buf) return 0;

    for (size_t i = 0; i < blen; i++) {
        int hi = hex_to_nybble(hex[2*i]);
        int lo = hex_to_nybble(hex[2*i+1]);
        if (hi < 0 || lo < 0) {
            OPENSSL_free(buf);
            return 0;
        }
        buf[i] = (unsigned char)((hi << 4) | lo);
    }

    *out = buf;
    *outlen = blen;
    return 1;
}

/* ---- Read whole file into NUL-terminated buffer ---- */
static char *read_all_text(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long sz = ftell(f);
    if (sz < 0) { fclose(f); return NULL; }
    if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return NULL; }

    char *buf = (char *)OPENSSL_malloc((size_t)sz + 1);
    if (!buf) { fclose(f); return NULL; }

    size_t got = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    if (got != (size_t)sz) { OPENSSL_free(buf); return NULL; }
    buf[sz] = '\0';
    return buf;
}

/* Find "KEY=" line and return pointer to value */
static const char *find_kv(const char *text, const char *key) {
    size_t klen = strlen(key);
    const char *p = text;
    while (*p) {
        const char *line = p;
        const char *eol = strchr(line, '\n');
        size_t linelen = eol ? (size_t)(eol - line) : strlen(line);

        if (linelen > klen + 1 &&
            strncmp(line, key, klen) == 0 &&
            line[klen] == '=') {
            return line + klen + 1; /* start of value */
        }
        if (!eol) break;
        p = eol + 1;
    }
    return NULL;
}

/* ---- SHA256 ---- */
static int sha256_bytes(const unsigned char *in, size_t inlen, unsigned char out32[32]) {
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

/* ---- HKDF-SHA256 → AES-128 key from ss ---- */
static int hkdf_sha256_key128(const unsigned char *ss, size_t ss_len,
                              unsigned char key_out[AES128_KEY_LEN]) {
    int ok = 0;
    EVP_KDF *kdf = NULL;
    EVP_KDF_CTX *kctx = NULL;

    static const unsigned char salt[] = "dc-multitool-salt";
    static const unsigned char info[] = "mlkem512-aes128gcm-data_contract";

    kdf = EVP_KDF_fetch(NULL, "HKDF", NULL);
    if (!kdf) goto end;
    kctx = EVP_KDF_CTX_new(kdf);
    if (!kctx) goto end;

    char digest_name[] = "SHA256";
    OSSL_PARAM params[] = {
        OSSL_PARAM_construct_utf8_string(OSSL_KDF_PARAM_DIGEST, digest_name, 0),
        OSSL_PARAM_construct_octet_string(OSSL_KDF_PARAM_KEY, (void *)ss, ss_len),
        OSSL_PARAM_construct_octet_string(OSSL_KDF_PARAM_SALT, (void *)salt, sizeof(salt) - 1),
        OSSL_PARAM_construct_octet_string(OSSL_KDF_PARAM_INFO, (void *)info, sizeof(info) - 1),
        OSSL_PARAM_construct_end()
    };

    ok = EVP_KDF_derive(kctx, key_out, AES128_KEY_LEN, params);

end:
    EVP_KDF_CTX_free(kctx);
    EVP_KDF_free(kdf);
    return ok;
}

/* ---- AES-128-GCM ---- */
static int aes_128_gcm_encrypt(const unsigned char key[AES128_KEY_LEN],
                               const unsigned char iv[GCM_IV_LEN],
                               const unsigned char *pt, size_t pt_len,
                               unsigned char *ct,
                               unsigned char tag[GCM_TAG_LEN]) {
    int ok = 0, len = 0, fin = 0;
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return 0;

    if (EVP_EncryptInit_ex(ctx, EVP_aes_128_gcm(), NULL, NULL, NULL) != 1) goto end;
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, GCM_IV_LEN, NULL) != 1) goto end;
    if (EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv) != 1) goto end;

    if (EVP_EncryptUpdate(ctx, ct, &len, pt, (int)pt_len) != 1) goto end;
    if (EVP_EncryptFinal_ex(ctx, ct + len, &fin) != 1) goto end;

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, GCM_TAG_LEN, tag) != 1) goto end;
    ok = 1;

end:
    EVP_CIPHER_CTX_free(ctx);
    return ok;
}

static int aes_128_gcm_decrypt(const unsigned char key[AES128_KEY_LEN],
                               const unsigned char iv[GCM_IV_LEN],
                               const unsigned char *ct, size_t ct_len,
                               const unsigned char tag[GCM_TAG_LEN],
                               unsigned char *pt_out) {
    int ok = 0, len = 0, fin = 0;
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return 0;

    if (EVP_DecryptInit_ex(ctx, EVP_aes_128_gcm(), NULL, NULL, NULL) != 1) goto end;
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, GCM_IV_LEN, NULL) != 1) goto end;
    if (EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv) != 1) goto end;

    if (EVP_DecryptUpdate(ctx, pt_out, &len, ct, (int)ct_len) != 1) goto end;
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, GCM_TAG_LEN, (void *)tag) != 1) goto end;

    if (EVP_DecryptFinal_ex(ctx, pt_out + len, &fin) != 1) goto end;

    ok = 1;
end:
    EVP_CIPHER_CTX_free(ctx);
    return ok;
}

/* ---- PEM helpers ---- */
static EVP_PKEY *read_pubkey_pem(const char *path) {
    BIO *bio = BIO_new_file(path, "rb");
    if (!bio) return NULL;
    EVP_PKEY *pkey = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL);
    BIO_free(bio);
    return pkey;
}
static EVP_PKEY *read_privkey_pem(const char *path) {
    BIO *bio = BIO_new_file(path, "rb");
    if (!bio) return NULL;
    EVP_PKEY *pkey = PEM_read_bio_PrivateKey(bio, NULL, NULL, NULL);
    BIO_free(bio);
    return pkey;
}
static int write_pubkey_pem(const char *path, EVP_PKEY *pkey) {
    BIO *bio = BIO_new_file(path, "wb");
    if (!bio) return 0;
    int ok = PEM_write_bio_PUBKEY(bio, pkey);
    BIO_free(bio);
    return ok;
}
static int write_privkey_pem_unencrypted(const char *path, EVP_PKEY *pkey) {
    BIO *bio = BIO_new_file(path, "wb");
    if (!bio) return 0;
    int ok = PEM_write_bio_PrivateKey(bio, pkey, NULL, NULL, 0, NULL, NULL);
    BIO_free(bio);
    return ok;
}

/* ---- Combined text file writer: KEM_CT + AEAD(Mojo-V data contract) ---- */
static void write_combined_file(const char *path,
                                const unsigned char *kem_ct, size_t kem_ct_len,
                                const unsigned char iv[GCM_IV_LEN],
                                const unsigned char tag[GCM_TAG_LEN],
                                const unsigned char *msg_ct, size_t msg_ct_len,
                                const unsigned char pt_sha[32]) {
    FILE *f = fopen(path, "wb");
    if (!f) die_msg("fopen(combined ct file) failed");

    char *kem_ct_hex = bytes_to_hex(kem_ct, kem_ct_len);
    char *iv_hex     = bytes_to_hex(iv, GCM_IV_LEN);
    char *tag_hex    = bytes_to_hex(tag, GCM_TAG_LEN);
    char *msg_ct_hex = bytes_to_hex(msg_ct, msg_ct_len);
    char *hash_hex   = bytes_to_hex(pt_sha, 32);

    if (!kem_ct_hex || !iv_hex || !tag_hex || !msg_ct_hex || !hash_hex)
        die_msg("bytes_to_hex failed");

    fprintf(f, "KEM=ML-KEM-512\n");
    fprintf(f, "AEAD=AES-128-GCM\n");
    fprintf(f, "KEM_CT=%s\n", kem_ct_hex);
    fprintf(f, "IV=%s\n", iv_hex);
    fprintf(f, "TAG=%s\n", tag_hex);
    fprintf(f, "MSG_CT=%s\n", msg_ct_hex);
    fprintf(f, "PT_SHA256=%s\n", hash_hex);

    OPENSSL_free(kem_ct_hex);
    OPENSSL_free(iv_hex);
    OPENSSL_free(tag_hex);
    OPENSSL_free(msg_ct_hex);
    OPENSSL_free(hash_hex);
    fclose(f);
}

/* ---- Steps ---- */

static void step_keygen(void) {
    OSSL_PROVIDER *prov = OSSL_PROVIDER_load(NULL, "default");
    if (!prov) die_openssl("OSSL_PROVIDER_load(default)");

    EVP_PKEY_CTX *kctx = EVP_PKEY_CTX_new_from_name(NULL, "ML-KEM-512", NULL);
    if (!kctx) die_openssl("EVP_PKEY_CTX_new_from_name(ML-KEM-512)");
    if (EVP_PKEY_keygen_init(kctx) != 1) die_openssl("EVP_PKEY_keygen_init");

    EVP_PKEY *kp = NULL;
    if (EVP_PKEY_keygen(kctx, &kp) != 1) die_openssl("EVP_PKEY_keygen");

    if (write_pubkey_pem(PK_FILE, kp) != 1) die_openssl("write_pubkey_pem");
    if (write_privkey_pem_unencrypted(SK_FILE, kp) != 1) die_openssl("write_privkey_pem_unencrypted");

    printf("INIT: wrote public key to %s and private key to %s\n", PK_FILE, SK_FILE);

    EVP_PKEY_free(kp);
    EVP_PKEY_CTX_free(kctx);
    OSSL_PROVIDER_unload(prov);
}

/* format_sel: 0=fast, 1=strong, 2=proofcarrying */
static void step_dcgen(uint8_t format_sel) {
    OSSL_PROVIDER *prov = OSSL_PROVIDER_load(NULL, "default");
    if (!prov) die_openssl("OSSL_PROVIDER_load(default)");

    EVP_PKEY *pk = read_pubkey_pem(PK_FILE);
    if (!pk) die_openssl("ALICE: read pk pem");

    /* --- KEM encapsulate --- */
    EVP_PKEY_CTX *ectx = EVP_PKEY_CTX_new_from_pkey(NULL, pk, NULL);
    if (!ectx) die_openssl("ALICE: EVP_PKEY_CTX_new_from_pkey(encap)");
    if (EVP_PKEY_encapsulate_init(ectx, NULL) != 1) die_openssl("ALICE: EVP_PKEY_encapsulate_init");

    size_t kem_ct_len = 0, ss_len = 0;
    if (EVP_PKEY_encapsulate(ectx, NULL, &kem_ct_len, NULL, &ss_len) != 1)
        die_openssl("ALICE: EVP_PKEY_encapsulate(size query)");

    unsigned char *kem_ct = (unsigned char *)OPENSSL_malloc(kem_ct_len);
    unsigned char *ss     = (unsigned char *)OPENSSL_malloc(ss_len);
    if (!kem_ct || !ss) die_msg("ALICE: malloc for KEM");

    if (EVP_PKEY_encapsulate(ectx, kem_ct, &kem_ct_len, ss, &ss_len) != 1)
        die_openssl("ALICE: EVP_PKEY_encapsulate");

    /* Derive AES-128 key from ss */
    unsigned char k[AES128_KEY_LEN];
    if (hkdf_sha256_key128(ss, ss_len, k) != 1)
        die_openssl("ALICE: HKDF derive key");

    /* Build Mojo-V data contract, encode to 64 bytes */
    data_contract_t dc;
    dc_init(&dc, format_sel);

    unsigned char pt[DC_WIRE_LEN];
    dc_encode(pt, &dc);

    unsigned char pt_sha[32];
    if (sha256_bytes(pt, sizeof(pt), pt_sha) != 1)
        die_openssl("ALICE: SHA256(data_contract pt)");

    /* Encrypt with AES-128-GCM */
    unsigned char iv[GCM_IV_LEN];
    if (RAND_bytes(iv, sizeof(iv)) != 1)
        die_openssl("ALICE: RAND_bytes(iv)");

    unsigned char msg_ct[DC_WIRE_LEN];
    unsigned char tag[GCM_TAG_LEN];
    if (aes_128_gcm_encrypt(k, iv, pt, sizeof(pt), msg_ct, tag) != 1)
        die_openssl("ALICE: AES-128-GCM encrypt");

    write_combined_file(CT_FILE, kem_ct, kem_ct_len,
                        iv, tag, msg_ct, sizeof(msg_ct), pt_sha);

    printf("ALICE: wrote combined KEM+contract file to %s\n", CT_FILE);

    /* Cleanup */
    OPENSSL_cleanse(k, sizeof(k));
    OPENSSL_cleanse(ss, ss_len);

    OPENSSL_free(ss);
    OPENSSL_free(kem_ct);

    EVP_PKEY_CTX_free(ectx);
    EVP_PKEY_free(pk);
    OSSL_PROVIDER_unload(prov);
}

static void step_dcchk(void) {
    OSSL_PROVIDER *prov = OSSL_PROVIDER_load(NULL, "default");
    if (!prov) die_openssl("OSSL_PROVIDER_load(default)");

    EVP_PKEY *sk = read_privkey_pem(SK_FILE);
    if (!sk) die_openssl("BOB: read sk pem");

    /* Read combined file */
    char *text = read_all_text(CT_FILE);
    if (!text) die_msg("BOB: read combined file failed");

    const char *kem_ct_hex = find_kv(text, "KEM_CT");
    const char *iv_hex     = find_kv(text, "IV");
    const char *tag_hex    = find_kv(text, "TAG");
    const char *msg_ct_hex = find_kv(text, "MSG_CT");
    const char *hsh_hex    = find_kv(text, "PT_SHA256");

    if (!kem_ct_hex || !iv_hex || !tag_hex || !msg_ct_hex || !hsh_hex)
        die_msg("BOB: missing fields in combined file");

    unsigned char *kem_ct = NULL, *iv_b = NULL, *tag_b = NULL, *msg_ct = NULL, *hsh_b = NULL;
    size_t kem_ct_len = 0, iv_len = 0, tag_len = 0, msg_ct_len = 0, hsh_len = 0;

    if (hex_to_bytes(kem_ct_hex, &kem_ct, &kem_ct_len) != 1)
        die_msg("BOB: parse KEM_CT hex failed");
    if (hex_to_bytes(iv_hex, &iv_b, &iv_len) != 1)
        die_msg("BOB: parse IV hex failed");
    if (hex_to_bytes(tag_hex, &tag_b, &tag_len) != 1)
        die_msg("BOB: parse TAG hex failed");
    if (hex_to_bytes(msg_ct_hex, &msg_ct, &msg_ct_len) != 1)
        die_msg("BOB: parse MSG_CT hex failed");
    if (hex_to_bytes(hsh_hex, &hsh_b, &hsh_len) != 1)
        die_msg("BOB: parse PT_SHA256 hex failed");

    if (iv_len != GCM_IV_LEN || tag_len != GCM_TAG_LEN ||
        msg_ct_len != DC_WIRE_LEN || hsh_len != 32)
        die_msg("BOB: invalid field lengths in combined file");

    /* Decapsulate KEM */
    EVP_PKEY_CTX *dctx = EVP_PKEY_CTX_new_from_pkey(NULL, sk, NULL);
    if (!dctx) die_openssl("BOB: EVP_PKEY_CTX_new_from_pkey(decap)");
    if (EVP_PKEY_decapsulate_init(dctx, NULL) != 1)
        die_openssl("BOB: EVP_PKEY_decapsulate_init");

    size_t ss_len = 0;
    if (EVP_PKEY_decapsulate(dctx, NULL, &ss_len, kem_ct, kem_ct_len) != 1)
        die_openssl("BOB: EVP_PKEY_decapsulate(size query)");

    unsigned char *ss = (unsigned char *)OPENSSL_malloc(ss_len);
    if (!ss) die_msg("BOB: malloc ss");

    if (EVP_PKEY_decapsulate(dctx, ss, &ss_len, kem_ct, kem_ct_len) != 1)
        die_openssl("BOB: EVP_PKEY_decapsulate");

    /* Derive AES-128 key and decrypt contract */
    unsigned char k[AES128_KEY_LEN];
    if (hkdf_sha256_key128(ss, ss_len, k) != 1)
        die_openssl("BOB: HKDF derive key");

    unsigned char pt[DC_WIRE_LEN];
    if (aes_128_gcm_decrypt(k, iv_b, msg_ct, msg_ct_len, tag_b, pt) != 1) {
        fprintf(stderr, "BOB: AES-128-GCM decrypt FAILED (tag / key mismatch)\n");
        goto cleanup;
    }

    unsigned char pt_sha[32];
    if (sha256_bytes(pt, sizeof(pt), pt_sha) != 1)
        die_openssl("BOB: SHA256(pt)");

    if (CRYPTO_memcmp(pt_sha, hsh_b, 32) != 0) {
        fprintf(stderr, "BOB: validation FAILED (PT_SHA256 mismatch)\n");
        goto cleanup;
    }

    data_contract_t dc;
    dc_decode(&dc, pt);

    /* Check header signature */
    static const char sig_str[16] = "Mojo-V ver. #001";
    if (CRYPTO_memcmp(dc.sig, sig_str, 16) != 0) {
        fprintf(stderr, "BOB: invalid data contract signature header\n");
        goto cleanup;
    }

    printf("SUCCESS: ALICE -> BOB Mojo-V data_contract_t transfer validated.\n");
    printf("Decrypted data_contract_t fields:\n");
    printf("  sig         = \"");
    for (int i = 0; i < 16; i++) putchar(dc.sig[i]);
    printf("\"\n");
    printf("  sym_key_128 = 0x");
    for (int i = 0; i < 16; i++) printf("%02x", dc.sym_key_128[i]);
    printf("\n");
    printf("  contract_sig= 0x%016llx\n", (unsigned long long)dc.contract_sig);
    printf("  salt        = 0x%016llx\n", (unsigned long long)dc.salt);
    printf("  ciphers     = 0x%016llx\n", (unsigned long long)dc.ciphers);
    printf("  format_sel  = %u", (unsigned)dc.format_sel);
    if      (dc.format_sel == 0) printf(" (fast)\n");
    else if (dc.format_sel == 1) printf(" (strong)\n");
    else if (dc.format_sel == 2) printf(" (proofcarrying)\n");
    else                         printf(" (unknown)\n");

cleanup:
    OPENSSL_cleanse(k, sizeof(k));
    OPENSSL_cleanse(ss, ss_len);

    OPENSSL_free(ss);
    OPENSSL_free(kem_ct);
    OPENSSL_free(iv_b);
    OPENSSL_free(tag_b);
    OPENSSL_free(msg_ct);
    OPENSSL_free(hsh_b);
    OPENSSL_free(text);

    EVP_PKEY_CTX_free(dctx);
    EVP_PKEY_free(sk);
    OSSL_PROVIDER_unload(prov);
}

/* ---- CLI ---- */

static void usage(const char *argv0) {
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  %s keygen\n", argv0);
    fprintf(stderr, "  %s dcgen {fast,strong,proof-carrying}\n", argv0);
    fprintf(stderr, "  %s dcchk\n", argv0);
    fprintf(stderr, "  %s all   # init + dcgen(fast) + dcchk\n", argv0);
    fprintf(stderr, "\nFiles:\n");
    fprintf(stderr, "  %s  (public key PEM)\n", PK_FILE);
    fprintf(stderr, "  %s  (private key PEM)\n", SK_FILE);
    fprintf(stderr, "  %s  (KEM ct + encrypted data contract)\n", CT_FILE);
    exit(2);
}

/* Map dcgen mode string to format_sel */
static uint8_t parse_format_sel(const char *mode) {
    if (strcmp(mode, "fast") == 0)
        return 0;
    if (strcmp(mode, "strong") == 0)
        return 1;
    if (strcmp(mode, "proof-carrying") == 0 || strcmp(mode, "proofcarrying") == 0)
        return 2;

    fprintf(stderr, "Unknown format mode '%s', expected {fast,strong,proof-carrying}\n",
            mode);
    exit(2);
}

int main(int argc, char **argv) {
    if (argc < 2) usage(argv[0]);

    ERR_load_crypto_strings();

    if (strcmp(argv[1], "keygen") == 0) {
        step_keygen();
    } else if (strcmp(argv[1], "dcgen") == 0) {
        if (argc < 3) {
            fprintf(stderr, "dcgen requires a mode: {fast,strong,proof-carrying}\n");
            usage(argv[0]);
        }
        uint8_t format_sel = parse_format_sel(argv[2]);
        step_dcgen(format_sel);
    } else if (strcmp(argv[1], "dcchk") == 0) {
        step_dcchk();
    } else if (strcmp(argv[1], "all") == 0) {
        /* Default all→fast */
        step_keygen();
        step_dcgen(0);
        step_dcchk();
    } else {
        usage(argv[0]);
    }

    return 0;
}

