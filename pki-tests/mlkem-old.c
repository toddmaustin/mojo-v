// mlkem_files_demo.c
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
#define CT_FILE  "mlkem512-ct.txt"
#define MSG_FILE "my-msg-enc.txt"

#define GCM_IV_LEN     12
#define GCM_TAG_LEN    16
#define AES128_KEY_LEN 16

/* ---- Example struct to transfer securely ---- */
typedef struct {
    uint32_t magic;
    uint64_t counter;
    uint8_t  flags;
    uint8_t  payload[16];
} my_msg_t;

/* Stable wire format (big-endian, no padding) */
#define MSG_WIRE_LEN (4 + 8 + 1 + 16)

static void die_openssl(const char *what) {
    fprintf(stderr, "ERROR: %s\n", what);
    ERR_print_errors_fp(stderr);
    exit(1);
}
static void die_msg(const char *what) {
    fprintf(stderr, "ERROR: %s\n", what);
    exit(1);
}

static void store_u32_be(unsigned char *dst, uint32_t v) {
    dst[0] = (unsigned char)((v >> 24) & 0xff);
    dst[1] = (unsigned char)((v >> 16) & 0xff);
    dst[2] = (unsigned char)((v >>  8) & 0xff);
    dst[3] = (unsigned char)((v >>  0) & 0xff);
}
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
static uint32_t load_u32_be(const unsigned char *src) {
    return ((uint32_t)src[0] << 24) |
           ((uint32_t)src[1] << 16) |
           ((uint32_t)src[2] <<  8) |
           ((uint32_t)src[3] <<  0);
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

static void msg_encode(unsigned char out[MSG_WIRE_LEN], const my_msg_t *m) {
    store_u32_be(out + 0,  m->magic);
    store_u64_be(out + 4,  m->counter);
    out[12] = m->flags;
    memcpy(out + 13, m->payload, 16);
}
static void msg_decode(my_msg_t *m, const unsigned char in[MSG_WIRE_LEN]) {
    m->magic   = load_u32_be(in + 0);
    m->counter = load_u64_be(in + 4);
    m->flags   = in[12];
    memcpy(m->payload, in + 13, 16);
}

/* ---- Hex helpers ---- */
static char nybble_to_hex(unsigned v) {
    return (v < 10) ? (char)('0' + v) : (char)('a' + (v - 10));
}
static int hex_to_nybble(char c) {
    if ('0' <= c && c <= '9') return c - '0';
    if ('a' <= c && c <= 'f') return 10 + (c - 'a');
    if ('A' <= c && c <= 'F') return 10 + (c - 'A');
    return -1;
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

static int hex_to_bytes(const char *hex, unsigned char **out, size_t *outlen)
{
    /* Only parse up to end-of-line (or NUL) */
    size_t n = 0;
    while (hex[n] != '\0' && hex[n] != '\n' && hex[n] != '\r')
        n++;

    /* Trim trailing spaces/tabs on this line */
    while (n > 0 && (hex[n-1] == ' ' || hex[n-1] == '\t'))
        n--;

    if (n == 0) return 0;
    if (n % 2 != 0) return 0;

    size_t blen = n / 2;
    unsigned char *buf = (unsigned char *)OPENSSL_malloc(blen);
    if (!buf) return 0;

    for (size_t i = 0; i < blen; i++) {
        int hi = hex_to_nybble(hex[2*i]);
        int lo = hex_to_nybble(hex[2*i + 1]);
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

/* Read whole file to a NUL-terminated buffer */
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

/* Find "KEY=" line; returns pointer to start of value (within text buffer) */
static const char *find_kv(const char *text, const char *key) {
    size_t klen = strlen(key);
    const char *p = text;
    while (*p) {
        const char *line = p;
        const char *eol = strchr(line, '\n');
        size_t linelen = eol ? (size_t)(eol - line) : strlen(line);

        if (linelen > klen + 1 && strncmp(line, key, klen) == 0 && line[klen] == '=') {
            return line + klen + 1; /* value */
        }
        if (!eol) break;
        p = eol + 1;
    }
    return NULL;
}

/* ---- SHA256 (via EVP) ---- */
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

/* ---- HKDF-SHA256 to derive AES-128 key from ss ---- */
static int hkdf_sha256_key128(const unsigned char *ss, size_t ss_len,
                              unsigned char key_out[AES128_KEY_LEN]) {
    int ok = 0;
    EVP_KDF *kdf = NULL;
    EVP_KDF_CTX *kctx = NULL;

    static const unsigned char salt[] = "mlkem-demo-salt";
    static const unsigned char info[] = "mlkem512-aes128gcm-my_msg_t";

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

    /* returns <=0 on tag failure */
    if (EVP_DecryptFinal_ex(ctx, pt_out + len, &fin) != 1) goto end;

    ok = 1;
end:
    EVP_CIPHER_CTX_free(ctx);
    return ok;
}

/* ---- File writers ---- */
static void write_text_kv_file_ct(const char *path, const unsigned char *ct, size_t ct_len) {
    FILE *f = fopen(path, "wb");
    if (!f) die_msg("fopen(ct file) failed");
    char *hex = bytes_to_hex(ct, ct_len);
    if (!hex) die_msg("bytes_to_hex(ct) failed");

    fprintf(f, "KEM=ML-KEM-512\n");
    fprintf(f, "CT=%s\n", hex);

    OPENSSL_free(hex);
    fclose(f);
}

static void write_text_kv_file_msg(const char *path,
                                  const unsigned char iv[GCM_IV_LEN],
                                  const unsigned char tag[GCM_TAG_LEN],
                                  const unsigned char *ct, size_t ct_len,
                                  const unsigned char pt_sha[32]) {
    FILE *f = fopen(path, "wb");
    if (!f) die_msg("fopen(msg file) failed");

    char *ivh  = bytes_to_hex(iv, GCM_IV_LEN);
    char *tagh = bytes_to_hex(tag, GCM_TAG_LEN);
    char *cth  = bytes_to_hex(ct, ct_len);
    char *hsh  = bytes_to_hex(pt_sha, 32);
    if (!ivh || !tagh || !cth || !hsh) die_msg("bytes_to_hex(msg parts) failed");

    fprintf(f, "AEAD=AES-128-GCM\n");
    fprintf(f, "IV=%s\n", ivh);
    fprintf(f, "TAG=%s\n", tagh);
    fprintf(f, "CT=%s\n", cth);
    fprintf(f, "PT_SHA256=%s\n", hsh);

    OPENSSL_free(ivh);
    OPENSSL_free(tagh);
    OPENSSL_free(cth);
    OPENSSL_free(hsh);
    fclose(f);
}

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
    /* Unencrypted PKCS#8 PEM (text-readable). For production, encrypt this file. */
    int ok = PEM_write_bio_PrivateKey(bio, pkey, NULL, NULL, 0, NULL, NULL);
    BIO_free(bio);
    return ok;
}

/* ---- Steps ---- */
static void step_init(void) {
    OSSL_PROVIDER *prov = OSSL_PROVIDER_load(NULL, "default");
    if (!prov) die_openssl("OSSL_PROVIDER_load(default)");

    EVP_PKEY_CTX *kctx = EVP_PKEY_CTX_new_from_name(NULL, "ML-KEM-512", NULL);
    if (!kctx) die_openssl("EVP_PKEY_CTX_new_from_name(ML-KEM-512)");
    if (EVP_PKEY_keygen_init(kctx) != 1) die_openssl("EVP_PKEY_keygen_init");

    EVP_PKEY *kp = NULL;
    if (EVP_PKEY_keygen(kctx, &kp) != 1) die_openssl("EVP_PKEY_keygen");

    if (write_pubkey_pem(PK_FILE, kp) != 1) die_openssl("write_pubkey_pem");
    if (write_privkey_pem_unencrypted(SK_FILE, kp) != 1) die_openssl("write_privkey_pem_unencrypted");

    printf("INIT: wrote public key to `%s' and private key to `%s'\n", PK_FILE, SK_FILE);

    EVP_PKEY_free(kp);
    EVP_PKEY_CTX_free(kctx);
    OSSL_PROVIDER_unload(prov);
}

static void step_alice(void) {
    OSSL_PROVIDER *prov = OSSL_PROVIDER_load(NULL, "default");
    if (!prov) die_openssl("OSSL_PROVIDER_load(default)");

    EVP_PKEY *pk = read_pubkey_pem(PK_FILE);
    if (!pk) die_openssl("ALICE: read pk pem");

    EVP_PKEY_CTX *ectx = EVP_PKEY_CTX_new_from_pkey(NULL, pk, NULL);
    if (!ectx) die_openssl("ALICE: EVP_PKEY_CTX_new_from_pkey(encap)");
    if (EVP_PKEY_encapsulate_init(ectx, NULL) != 1) die_openssl("ALICE: EVP_PKEY_encapsulate_init");

    size_t kem_ct_len = 0, ss_len = 0;
    if (EVP_PKEY_encapsulate(ectx, NULL, &kem_ct_len, NULL, &ss_len) != 1)
        die_openssl("ALICE: EVP_PKEY_encapsulate(size query)");

    unsigned char *kem_ct = (unsigned char *)OPENSSL_malloc(kem_ct_len);
    unsigned char *ss     = (unsigned char *)OPENSSL_malloc(ss_len);
    if (!kem_ct || !ss) die_msg("ALICE: malloc");

    if (EVP_PKEY_encapsulate(ectx, kem_ct, &kem_ct_len, ss, &ss_len) != 1)
        die_openssl("ALICE: EVP_PKEY_encapsulate");

    write_text_kv_file_ct(CT_FILE, kem_ct, kem_ct_len);
    printf("ALICE: wrote encapsulated secret (ss, %lu bytes) to ciphertext (ct, %lu bytes) file `%s'\n", ss_len, kem_ct_len, CT_FILE);

    /* Derive AES-128 key k from ss */
    unsigned char k[AES128_KEY_LEN];
    if (hkdf_sha256_key128(ss, ss_len, k) != 1) die_openssl("ALICE: HKDF derive key");
    printf("ALICE: derived AES key (k, %u bytes) from encapsulated secret (ss, %lu bytes)'\n", AES128_KEY_LEN, ss_len);

    /* Create plaintext message, serialize, hash, encrypt */
    my_msg_t msg = {0};
    msg.magic = 0x4D4C4B45; /* 'MLKE' */
    msg.counter = 12345;
    msg.flags = 0x5A;
    if (RAND_bytes(msg.payload, sizeof(msg.payload)) != 1) die_openssl("ALICE: RAND_bytes(payload)");

    unsigned char pt[MSG_WIRE_LEN];
    msg_encode(pt, &msg);

    unsigned char pt_sha[32];
    if (sha256_bytes(pt, sizeof(pt), pt_sha) != 1) die_openssl("ALICE: SHA256(pt)");

    unsigned char iv[GCM_IV_LEN];
    if (RAND_bytes(iv, sizeof(iv)) != 1) die_openssl("ALICE: RAND_bytes(iv)");

    unsigned char ct_msg[MSG_WIRE_LEN];
    unsigned char tag[GCM_TAG_LEN];
    if (aes_128_gcm_encrypt(k, iv, pt, sizeof(pt), ct_msg, tag) != 1)
        die_openssl("ALICE: AES-128-GCM encrypt");

    write_text_kv_file_msg(MSG_FILE, iv, tag, ct_msg, sizeof(ct_msg), pt_sha);
    printf("ALICE: wrote encrypted my_msg_t package to %s\n", MSG_FILE);

    /* Cleanup */
    OPENSSL_cleanse(k, sizeof(k));
    OPENSSL_cleanse(ss, ss_len);

    OPENSSL_free(ss);
    OPENSSL_free(kem_ct);

    EVP_PKEY_CTX_free(ectx);
    EVP_PKEY_free(pk);
    OSSL_PROVIDER_unload(prov);
}

static void step_bob(void) {
    OSSL_PROVIDER *prov = OSSL_PROVIDER_load(NULL, "default");
    if (!prov) die_openssl("OSSL_PROVIDER_load(default)");

    EVP_PKEY *sk = read_privkey_pem(SK_FILE);
    if (!sk) die_openssl("BOB: read sk pem");

    /* Read ct file */
    char *ct_text = read_all_text(CT_FILE);
    if (!ct_text) die_msg("BOB: read ct file failed");
    const char *ct_hex = find_kv(ct_text, "CT");
    if (!ct_hex) die_msg("BOB: CT= not found in ct file");

    unsigned char *kem_ct = NULL;
    size_t kem_ct_len = 0;
    if (hex_to_bytes(ct_hex, &kem_ct, &kem_ct_len) != 1) die_msg("BOB: parse CT hex failed");

    /* Decapsulate to recover ss */
    EVP_PKEY_CTX *dctx = EVP_PKEY_CTX_new_from_pkey(NULL, sk, NULL);
    if (!dctx) die_openssl("BOB: EVP_PKEY_CTX_new_from_pkey(decap)");
    if (EVP_PKEY_decapsulate_init(dctx, NULL) != 1) die_openssl("BOB: EVP_PKEY_decapsulate_init");

    size_t ss_len = 0;
    if (EVP_PKEY_decapsulate(dctx, NULL, &ss_len, kem_ct, kem_ct_len) != 1)
        die_openssl("BOB: EVP_PKEY_decapsulate(size query)");

    unsigned char *ss = (unsigned char *)OPENSSL_malloc(ss_len);
    if (!ss) die_msg("BOB: malloc ss");

    if (EVP_PKEY_decapsulate(dctx, ss, &ss_len, kem_ct, kem_ct_len) != 1)
        die_openssl("BOB: EVP_PKEY_decapsulate");

    /* Read encrypted msg file */
    char *msg_text = read_all_text(MSG_FILE);
    if (!msg_text) die_msg("BOB: read msg file failed");

    const char *iv_hex  = find_kv(msg_text, "IV");
    const char *tag_hex = find_kv(msg_text, "TAG");
    const char *mct_hex = find_kv(msg_text, "CT");
    const char *hsh_hex = find_kv(msg_text, "PT_SHA256");
    if (!iv_hex || !tag_hex || !mct_hex || !hsh_hex) die_msg("BOB: missing fields in msg file");

    unsigned char *iv_b = NULL, *tag_b = NULL, *mct_b = NULL, *hsh_b = NULL;
    size_t iv_len=0, tag_len=0, mct_len=0, hsh_len=0;

    if (hex_to_bytes(iv_hex,  &iv_b,  &iv_len)  != 1) die_msg("BOB: parse IV hex failed");
    if (hex_to_bytes(tag_hex, &tag_b, &tag_len) != 1) die_msg("BOB: parse TAG hex failed");
    if (hex_to_bytes(mct_hex, &mct_b, &mct_len) != 1) die_msg("BOB: parse msg CT hex failed");
    if (hex_to_bytes(hsh_hex, &hsh_b, &hsh_len) != 1) die_msg("BOB: parse PT_SHA256 hex failed");

    if (iv_len != GCM_IV_LEN || tag_len != GCM_TAG_LEN || mct_len != MSG_WIRE_LEN || hsh_len != 32)
        die_msg("BOB: invalid field lengths in msg file");

    /* Derive AES-128 key k from ss, decrypt */
    unsigned char k[AES128_KEY_LEN];
    if (hkdf_sha256_key128(ss, ss_len, k) != 1) die_openssl("BOB: HKDF derive key");

    unsigned char pt[MSG_WIRE_LEN];
    if (aes_128_gcm_decrypt(k, iv_b, mct_b, mct_len, tag_b, pt) != 1) {
        fprintf(stderr, "BOB: decrypt failed (tag mismatch / wrong key / corrupted file)\n");
        goto cleanup;
    }

    /* Validate transfer using PT_SHA256 from Alice */
    unsigned char pt_sha[32];
    if (sha256_bytes(pt, sizeof(pt), pt_sha) != 1) die_openssl("BOB: SHA256(pt)");

    if (CRYPTO_memcmp(pt_sha, hsh_b, 32) != 0) {
        fprintf(stderr, "BOB: validation FAILED (PT_SHA256 mismatch)\n");
        goto cleanup;
    }

    /* Decode and sanity-check */
    my_msg_t msg = {0};
    msg_decode(&msg, pt);
    if (msg.magic != 0x4D4C4B45) {
        fprintf(stderr, "BOB: validation FAILED (bad magic)\n");
        goto cleanup;
    }

    printf("SUCCESS: ALICE -> BOB transfer validated (AES-GCM + PT_SHA256).\n");
    printf("Decrypted my_msg_t:\n");
    printf("  magic   = 0x%08x\n", msg.magic);
    printf("  counter = %llu\n", (unsigned long long)msg.counter);
    printf("  flags   = 0x%02x\n", msg.flags);

cleanup:
    OPENSSL_cleanse(k, sizeof(k));
    OPENSSL_cleanse(ss, ss_len);

    OPENSSL_free(ss);
    OPENSSL_free(kem_ct);

    OPENSSL_free(iv_b);
    OPENSSL_free(tag_b);
    OPENSSL_free(mct_b);
    OPENSSL_free(hsh_b);

    OPENSSL_free(ct_text);
    OPENSSL_free(msg_text);

    EVP_PKEY_CTX_free(dctx);
    EVP_PKEY_free(sk);

    OSSL_PROVIDER_unload(prov);
}

static void usage(const char *argv0) {
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  %s init   | alice | bob | all\n", argv0);
    fprintf(stderr, "\nDefault files:\n");
    fprintf(stderr, "  %s (public key PEM)\n", PK_FILE);
    fprintf(stderr, "  %s (private key PEM)\n", SK_FILE);
    fprintf(stderr, "  %s (KEM ct text)\n", CT_FILE);
    fprintf(stderr, "  %s (encrypted my_msg_t text)\n", MSG_FILE);
    exit(2);
}

int main(int argc, char **argv) {
    if (argc < 2) usage(argv[0]);

    /* Helpful in demos */
    ERR_load_crypto_strings();

    if (strcmp(argv[1], "init") == 0) {
        step_init();
    } else if (strcmp(argv[1], "alice") == 0) {
        step_alice();
    } else if (strcmp(argv[1], "bob") == 0) {
        step_bob();
    } else if (strcmp(argv[1], "all") == 0) {
        step_init();
        step_alice();
        step_bob();
    } else {
        usage(argv[0]);
    }
    return 0;
}

