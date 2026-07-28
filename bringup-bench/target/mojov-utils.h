#ifndef MOJOV_UTILS_H
#define MOJOV_UTILS_H

#ifdef __cplusplus
/* libmin exposes a C interface even when included from C++ code. */
extern "C" {
#endif

#include <stdint.h>
#include "libmin.h"
#include "simon.h"

typedef unsigned __int128 uint128_t;

// Mojo-V asm instruction definitions (using the format-friendly .insn directive in GNU AS)
#define LDE(rd,base,ofs) ".insn i 0xb, 0x0, " #rd ", " #base ", " #ofs "\n\t"
#define SDE(src,base,ofs) ".insn s 0xb, 0x1, " #src ", " #ofs "(" #base ")\n\t"
#define FLDE(rd,base,ofs) ".insn i 0xb, 0x2, " #rd ", " #base ", " #ofs "\n\t"
#define FSDE(src,base,ofs) ".insn s 0xb, 0x3, " #src ", " #ofs "(" #base ")\n\t"
#define DISC(rd,rs1,rs2) ".insn r 0xb, 0x0, 0x1, " #rd ", " #rs1 ", " #rs2 "\n\t"
#define FDISC(rd,rs1,rs2) ".insn r 0xb, 0x1, 0x1, " #rd ", " #rs1 ", " #rs2 "\n\t"
#define CERTRNG(rd,site_id) ".insn i 0xb, 0x4, " #rd ", x0, " #site_id "\n\t"

#define MOJOV_HASH64_BASIS 0x9e3779b97f4a7c15ull

static inline uint64_t __instret(void)
{
  uint64_t insts;
  __asm__ volatile ("rdinstret %0" : "=r"(insts));
  return insts;
}

static inline uint64_t mojov_hash64_init(void)
{
  return MOJOV_HASH64_BASIS;
}

static inline uint64_t mojov_hash64(uint64_t h, uint64_t v)
{
  v += 0x9e3779b97f4a7c15ull;
  v = (v ^ (v >> 30)) * 0xbf58476d1ce4e5b9ull;
  v = (v ^ (v >> 27)) * 0x94d049bb133111ebull;
  v ^= (v >> 31);
  return h ^ v;
}

typedef union mojov_mem_fast_u64_t {
  uint128_t ct;
  struct {
    uint64_t val;
    uint32_t salt;
    uint32_t sig;
  } pt;
} mojov_mem_fast_u64_t;

typedef union mojov_mem_fast_fp64_t {
  uint128_t ct;
  struct {
    double val;
    uint32_t salt;
    uint32_t sig;
  } pt;
} mojov_mem_fast_fp64_t;

typedef union mojov_mem_strong_u64_t {
  struct {
    uint128_t ct_lo;
    uint128_t ct_hi;
  } ct;
  struct {
    uint64_t val;
    uint64_t salt;
    uint64_t sig;
    uint64_t metadata;
  } pt;
} mojov_mem_strong_u64_t;

typedef union mojov_mem_strong_fp64_t {
  struct {
    uint128_t ct_lo;
    uint128_t ct_hi;
  } ct;
  struct {
    double val;
    uint64_t salt;
    uint64_t sig;
    uint64_t metadata;
  } pt;
} mojov_mem_strong_fp64_t;

typedef union mojov_mem_proofcarrying_u64_t {
  struct {
    uint128_t ct_lo;
    uint128_t ct_hi;
  } ct;
  struct {
    uint64_t val;
    uint64_t salt;
    uint64_t sig;
    uint64_t metadata;
  } pt;
} mojov_mem_proofcarrying_u64_t;

typedef union mojov_mem_proofcarrying_fp64_t {
  struct {
    uint128_t ct_lo;
    uint128_t ct_hi;
  } ct;
  struct {
    double val;
    uint64_t salt;
    uint64_t sig;
    uint64_t metadata;
  } pt;
} mojov_mem_proofcarrying_fp64_t;

typedef union mojov_mem_datagrant_t {
  struct {
    uint128_t ct_lo;
    uint128_t ct_hi;
  } ct;
  struct {
    uint64_t dfhash;
    uint64_t salt;
    uint64_t sig;
    uint64_t metadata;
  } pt;
} mojov_mem_datagrant_t;

static inline uint64_t mojov_decrypt_fast_u64(simon_state_t *simon_state, mojov_mem_fast_u64_t ctval, uint64_t sig)
{
  mojov_mem_fast_u64_t ptval;
  simon_128_128_decrypt(simon_state, ctval.ct, &ptval.ct);
  if (ptval.pt.sig != (uint32_t)sig)
  {
    libmin_printf("ERROR: fast u64 decryption validation failed! (sig == 0x%08lx, expected == 0x%08x).\n", ptval.pt.sig, (uint32_t)sig);
    libmin_fail(-1);
  }
  return ptval.pt.val;
}

static inline int64_t mojov_decrypt_fast_i64(simon_state_t *simon_state, mojov_mem_fast_u64_t ctval, uint64_t sig)
{
  return (int64_t)mojov_decrypt_fast_u64(simon_state, ctval, sig);
}

static inline double mojov_decrypt_fast_fp64(simon_state_t *simon_state, mojov_mem_fast_fp64_t ctval, uint64_t sig)
{
  mojov_mem_fast_fp64_t ptval;
  simon_128_128_decrypt(simon_state, ctval.ct, &ptval.ct);
  if (ptval.pt.sig != (uint32_t)sig)
  {
    libmin_printf("ERROR: fast fp64 decryption validation failed! (sig == 0x%08lx, expected == 0x%08lx).\n", ptval.pt.sig, sig);
    libmin_fail(-1);
  }
  return ptval.pt.val;
}

static inline uint64_t mojov_decrypt_strong_u64(simon_state_t *simon_state, mojov_mem_strong_u64_t ctval, uint64_t sig)
{
  mojov_mem_strong_u64_t ptval;
  simon_128_128_decrypt(simon_state, ctval.ct.ct_lo, &ptval.ct.ct_lo);
  simon_128_128_decrypt(simon_state, ctval.ct.ct_hi, &ptval.ct.ct_hi);
  ptval.ct.ct_hi ^= ctval.ct.ct_lo;
  if (ptval.pt.sig != sig)
  {
    libmin_printf("ERROR: strong u64 decryption validation failed! (sig == 0x%08lx, expected == 0x%08lx).\n", ptval.pt.sig, sig);
    libmin_printf("ERROR: val=%lu, sig=0x%08lx.\n", ptval.pt.val, ptval.pt.sig);
    libmin_fail(-1);
  }
  return ptval.pt.val;
}

static inline double mojov_decrypt_strong_fp64(simon_state_t *simon_state, mojov_mem_strong_fp64_t ctval, uint64_t sig)
{
  mojov_mem_strong_fp64_t ptval;
  simon_128_128_decrypt(simon_state, ctval.ct.ct_lo, &ptval.ct.ct_lo);
  simon_128_128_decrypt(simon_state, ctval.ct.ct_hi, &ptval.ct.ct_hi);
  ptval.ct.ct_hi ^= ctval.ct.ct_lo;
  if (ptval.pt.sig != sig)
  {
    libmin_printf("ERROR: strong fp64 decryption validation failed! (sig == 0x%08lx, expected == 0x%08lx).\n", ptval.pt.sig, sig);
    libmin_fail(-1);
  }
  return ptval.pt.val;
}

static inline mojov_mem_proofcarrying_fp64_t
mojov_encrypt_proofcarrying_fp64(simon_state_t *simon_state, double dblval, uint64_t in_brand, uint64_t sig)
{
  uint64_t dfhash = mojov_hash64(mojov_hash64_init(), in_brand);
  mojov_mem_proofcarrying_fp64_t ptval = {.pt = { dblval, ((uint64_t)libmin_rand() << 32) | (uint64_t)libmin_rand(), sig, dfhash} };
  mojov_mem_proofcarrying_fp64_t ctval;

  // encrypt the memory packet with the processor's internal key
  simon_128_128_encrypt(simon_state, ptval.ct.ct_lo, &ctval.ct.ct_lo);
  simon_128_128_encrypt(simon_state, (ptval.ct.ct_hi ^ ctval.ct.ct_lo), &ctval.ct.ct_hi);

  return ctval;
}

static inline uint64_t mojov_decrypt_proofcarrying_u64(simon_state_t *simon_state, mojov_mem_proofcarrying_u64_t ctval, uint64_t sig)
{
  mojov_mem_proofcarrying_u64_t ptval;
  simon_128_128_decrypt(simon_state, ctval.ct.ct_lo, &ptval.ct.ct_lo);
  simon_128_128_decrypt(simon_state, ctval.ct.ct_hi, &ptval.ct.ct_hi);
  ptval.ct.ct_hi ^= ctval.ct.ct_lo;
  if (ptval.pt.sig != sig)
  {
    libmin_printf("ERROR: proofcarrying u64 decryption validation failed! (sig == 0x%08lx, expected == 0x%08lx).\n", ptval.pt.sig, sig);
    libmin_fail(-1);
  }
  return ptval.pt.val;
}

static inline double mojov_decrypt_proofcarrying_fp64(simon_state_t *simon_state, mojov_mem_proofcarrying_fp64_t ctval, uint64_t sig)
{
  mojov_mem_proofcarrying_fp64_t ptval;
  simon_128_128_decrypt(simon_state, ctval.ct.ct_lo, &ptval.ct.ct_lo);
  simon_128_128_decrypt(simon_state, ctval.ct.ct_hi, &ptval.ct.ct_hi);
  ptval.ct.ct_hi ^= ctval.ct.ct_lo;
  if (ptval.pt.sig != sig)
  {
    libmin_printf("ERROR: proofcarrying fp64 decryption validation failed! (sig == 0x%08lx, expected == 0x%08lx).\n", ptval.pt.sig, sig);
    libmin_fail(-1);
  }
  return ptval.pt.val;
}

static inline uint64_t mojov_dfhash_proofcarrying_fp64(simon_state_t *simon_state, mojov_mem_proofcarrying_fp64_t ctval, uint64_t sig)
{
  mojov_mem_proofcarrying_fp64_t ptval;
  simon_128_128_decrypt(simon_state, ctval.ct.ct_lo, &ptval.ct.ct_lo);
  simon_128_128_decrypt(simon_state, ctval.ct.ct_hi, &ptval.ct.ct_hi);
  ptval.ct.ct_hi ^= ctval.ct.ct_lo;
  if (ptval.pt.sig != sig)
  {
    libmin_printf("ERROR: decryption validation failed! (sig == 0x%08lx, expected == 0x%08lx).\n", ptval.pt.sig, sig);
    libmin_fail(-1);
  }
  return ptval.pt.metadata;
}

static inline void mojov_print_fast_128(uint128_t ct)
{
  libmin_printf("0x%08x%08x%08x%08x",
    (uint32_t)(ct >> 96),
    (uint32_t)(ct >> 64),
    (uint32_t)(ct >> 32),
    (uint32_t)ct);
}

static inline void mojov_print_256(uint128_t ct_hi, uint128_t ct_lo)
{
  libmin_printf("0x%08x%08x%08x%08x",
    (uint32_t)(ct_hi >> 96),
    (uint32_t)(ct_hi >> 64),
    (uint32_t)(ct_hi >> 32),
    (uint32_t)ct_hi);
  libmin_printf("%08x%08x%08x%08x",
    (uint32_t)(ct_lo >> 96),
    (uint32_t)(ct_lo >> 64),
    (uint32_t)(ct_lo >> 32),
    (uint32_t)ct_lo);
}

static inline void mojov_print_fast_u64(mojov_mem_fast_u64_t ct)
{
  mojov_print_fast_128(ct.ct);
}

static inline void mojov_print_fast_fp64(mojov_mem_fast_fp64_t ct)
{
  mojov_print_fast_128(ct.ct);
}

static inline void mojov_print_strong_u64(mojov_mem_strong_u64_t ct)
{
  mojov_print_256(ct.ct.ct_hi, ct.ct.ct_lo);
}

static inline void mojov_print_strong_fp64(mojov_mem_strong_fp64_t ct)
{
  mojov_print_256(ct.ct.ct_hi, ct.ct.ct_lo);
}

static inline void mojov_print_proofcarrying_u64(mojov_mem_proofcarrying_u64_t ct)
{
  mojov_print_256(ct.ct.ct_hi, ct.ct.ct_lo);
}

static inline void mojov_print_proofcarrying_fp64(mojov_mem_proofcarrying_fp64_t ct)
{
  mojov_print_256(ct.ct.ct_hi, ct.ct.ct_lo);
}

#if 0
#define secret_print(ct) _Generic((ct),   uint128_t: mojov_print_fast_128,   mojov_mem_fast_u64_t: mojov_print_fast_u64,   mojov_mem_fast_fp64_t: mojov_print_fast_fp64,   mojov_mem_strong_u64_t: mojov_print_strong_u64,   mojov_mem_strong_fp64_t: mojov_print_strong_fp64,   mojov_mem_proofcarrying_u64_t: mojov_print_proofcarrying_u64,   mojov_mem_proofcarrying_fp64_t: mojov_print_proofcarrying_fp64 )(ct)
#endif

void mojov_print_mojov_cfg(uint64_t val);
uint64_t mojov_read_mojov_cfg(void);
void mojov_write_mojov_cfg(uint64_t value);
uint64_t mojov_read_mojov_ciphers(void);
int mojov_enable_and_verify(void);

/* Backward compatibility aliases. */
void mojov_print_mprivregcfg(uint64_t val);
uint64_t mojov_read_mprivregcfg(void);
void mojov_write_mprivregcfg(uint64_t value);

int mojov_configure_kmsm_from_dc_fast(void);
int mojov_configure_kmsm_from_dc_strong(void);
int mojov_configure_kmsm_from_dc_proof(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif
