#ifndef MOJOV_UTILS_H
#define MOJOV_UTILS_H

#include <stdint.h>
#include "libmin.h"
#include "simon.h"

typedef unsigned __int128 uint128_t;

// Mojo-V asm instruction definitions (using the format-friendly .insn directive in GNU AS)
#define LDE(rd,base,ofs) ".insn i 0xb, 0x0, " #rd ", " #base ", " #ofs "\n\t"
#define SDE(src,base,ofs) ".insn s 0xb, 0x1, " #src ", " #ofs "(" #base ")\n\t"
#define FLDE(rd,base,ofs) ".insn i 0xb, 0x2, " #rd ", " #base ", " #ofs "\n\t"
#define FSDE(src,base,ofs) ".insn s 0xb, 0x3, " #src ", " #ofs "(" #base ")\n\t"

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
    uint64_t auth_sig;
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
    uint64_t auth_sig;
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
    uint64_t auth_sig;
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
    uint64_t auth_sig;
    uint64_t metadata;
  } pt;
} mojov_mem_proofcarrying_fp64_t;

static inline uint64_t mojov_decrypt_fast_u64(simon_state_t *simon_state, mojov_mem_fast_u64_t ctval)
{
  mojov_mem_fast_u64_t ptval;
  simon_128_128_decrypt(simon_state, ctval.ct, &ptval.ct);
  return ptval.pt.val;
}

static inline double mojov_decrypt_fast_fp64(simon_state_t *simon_state, mojov_mem_fast_fp64_t ctval)
{
  mojov_mem_fast_fp64_t ptval;
  simon_128_128_decrypt(simon_state, ctval.ct, &ptval.ct);
  return ptval.pt.val;
}

static inline uint64_t mojov_decrypt_strong_u64(simon_state_t *simon_state, mojov_mem_strong_u64_t ctval)
{
  mojov_mem_strong_u64_t ptval;
  simon_128_128_decrypt(simon_state, ctval.ct.ct_lo, &ptval.ct.ct_lo);
  simon_128_128_decrypt(simon_state, ctval.ct.ct_hi, &ptval.ct.ct_hi);
  ptval.ct.ct_hi ^= ctval.ct.ct_lo;
  return ptval.pt.val;
}

static inline double mojov_decrypt_strong_fp64(simon_state_t *simon_state, mojov_mem_strong_fp64_t ctval)
{
  mojov_mem_strong_fp64_t ptval;
  simon_128_128_decrypt(simon_state, ctval.ct.ct_lo, &ptval.ct.ct_lo);
  simon_128_128_decrypt(simon_state, ctval.ct.ct_hi, &ptval.ct.ct_hi);
  ptval.ct.ct_hi ^= ctval.ct.ct_lo;
  return ptval.pt.val;
}

static inline uint64_t mojov_decrypt_proofcarrying_u64(simon_state_t *simon_state, mojov_mem_proofcarrying_u64_t ctval)
{
  mojov_mem_proofcarrying_u64_t ptval;
  simon_128_128_decrypt(simon_state, ctval.ct.ct_lo, &ptval.ct.ct_lo);
  simon_128_128_decrypt(simon_state, ctval.ct.ct_hi, &ptval.ct.ct_hi);
  ptval.ct.ct_hi ^= ctval.ct.ct_lo;
  return ptval.pt.val;
}

static inline double mojov_decrypt_proofcarrying_fp64(simon_state_t *simon_state, mojov_mem_proofcarrying_fp64_t ctval)
{
  mojov_mem_proofcarrying_fp64_t ptval;
  simon_128_128_decrypt(simon_state, ctval.ct.ct_lo, &ptval.ct.ct_lo);
  simon_128_128_decrypt(simon_state, ctval.ct.ct_hi, &ptval.ct.ct_hi);
  ptval.ct.ct_hi ^= ctval.ct.ct_lo;
  return ptval.pt.val;
}

static inline uint64_t mojov_dfhash_proofcarrying_fp64(simon_state_t *simon_state, mojov_mem_proofcarrying_fp64_t ctval)
{
  mojov_mem_proofcarrying_fp64_t ptval;
  simon_128_128_decrypt(simon_state, ctval.ct.ct_lo, &ptval.ct.ct_lo);
  simon_128_128_decrypt(simon_state, ctval.ct.ct_hi, &ptval.ct.ct_hi);
  ptval.ct.ct_hi ^= ctval.ct.ct_lo;
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

#define secret_print(ct) _Generic((ct),   uint128_t: mojov_print_fast_128,   mojov_mem_fast_u64_t: mojov_print_fast_u64,   mojov_mem_fast_fp64_t: mojov_print_fast_fp64,   mojov_mem_strong_u64_t: mojov_print_strong_u64,   mojov_mem_strong_fp64_t: mojov_print_strong_fp64,   mojov_mem_proofcarrying_u64_t: mojov_print_proofcarrying_u64,   mojov_mem_proofcarrying_fp64_t: mojov_print_proofcarrying_fp64 )(ct)

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

#endif
