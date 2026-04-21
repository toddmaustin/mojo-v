#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"

#define BLOOM_BITS 256u
#define INSERT_COUNT 32u
#define QUERY_COUNT 64u
#define HASH_FUNCS 3u

static uint64e_t bloom_filter[BLOOM_BITS];

static inline uint64_t
hash_mix1(uint64_t x)
{
  x ^= x >> 33;
  x *= 0xff51afd7ed558ccdULL;
  x ^= x >> 33;
  x *= 0xc4ceb9fe1a85ec53ULL;
  x ^= x >> 33;
  return x;
}

static inline uint64_t
hash_mix2(uint64_t x)
{
  x ^= x >> 30;
  x *= 0xbf58476d1ce4e5b9ULL;
  x ^= x >> 27;
  x *= 0x94d049bb133111ebULL;
  x ^= x >> 31;
  return x;
}

static inline uint64_t
hash_mix3(uint64_t x)
{
  x ^= x << 13;
  x ^= x >> 7;
  x ^= x << 17;
  return x * 0x9e3779b97f4a7c15ULL;
}

static inline uint64_t
bloom_index(uint64_t key, unsigned which)
{
  uint64_t h = 0;
  if (which == 0)
    h = hash_mix1(key + 0x123456789ULL);
  else if (which == 1)
    h = hash_mix2(key + 0x9abcdef01ULL);
  else
    h = hash_mix3(key + 0xdeadbeef1ULL);

  return h % BLOOM_BITS;
}

static void
bloom_insert(uint64_t key)
{
  for (unsigned i = 0; i < HASH_FUNCS; ++i)
  {
    uint64_t idx = bloom_index(key, i);
    bloom_filter[idx] = 1;
  }
}

static uint64e_t
bloom_maybe_contains(uint64_t key)
{
  uint64e_t present = 1;
  for (unsigned i = 0; i < HASH_FUNCS; ++i)
  {
    uint64_t idx = bloom_index(key, i);
    present = present && (bloom_filter[idx] != 0);
  }
  return present;
}

int
main(void)
{
  if (mojov_configure_kmsm_from_dc_fast() != 0)
    return -1;

  if (mojov_enable_and_verify() != 0)
    return -1;

  if (debug_context(SIMON128_KEY, CONTRACT_SIG) != 0)
    return -1;

  for (unsigned i = 0; i < BLOOM_BITS; ++i)
    bloom_filter[i] = 0;

  uint64_t inserted_keys[INSERT_COUNT];
  for (unsigned i = 0; i < INSERT_COUNT; ++i)
  {
    inserted_keys[i] = 1000 + (uint64_t)(i * 7);
    bloom_insert(inserted_keys[i]);
  }

  unsigned true_positives = 0;
  for (unsigned i = 0; i < INSERT_COUNT; ++i)
  {
    if (bloom_maybe_contains(inserted_keys[i]).decrypt())
      ++true_positives;
  }

  uint64_t query_keys[QUERY_COUNT];
  unsigned false_positives = 0;
  unsigned true_negatives = 0;

  for (unsigned i = 0; i < QUERY_COUNT; ++i)
  {
    query_keys[i] = 50000 + (uint64_t)(i * 13);

    uint64_t maybe = bloom_maybe_contains(query_keys[i]).decrypt();
    if (maybe)
      ++false_positives;
    else
      ++true_negatives;
  }

  libmin_printf("Bloom filter benchmark:\n");
  libmin_printf("  bits=%u inserts=%u hash_functions=%u\n", BLOOM_BITS, INSERT_COUNT, HASH_FUNCS);
  libmin_printf("  true_positives=%u/%u\n", true_positives, INSERT_COUNT);
  libmin_printf("  true_negatives=%u/%u\n", true_negatives, QUERY_COUNT);
  libmin_printf("  false_positives=%u/%u\n", false_positives, QUERY_COUNT);

  if (true_positives != INSERT_COUNT)
  {
    libmin_printf("ERROR: false negatives detected\n");
    return -1;
  }

  libmin_success();
  return 0;
}
