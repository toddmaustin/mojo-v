#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"
#include "dc-proofcarrying.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_proofcarrying_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_proofcarrying_fp64_t
#include "mojov-exo.h"

using namespace exo;
static const unsigned sample_count = 16384;

int main(void)
{
  if (mojov_configure_kmsm_from_dc_proof() != 0 ||
      mojov_enable_and_verify() != 0 ||
      debug_context(SIMON128_KEY, CONTRACT_SIG) != 0)
    return -1;

  uint64e_t samples[sample_count];

  // Two inexpensive aggregate checks are computed while every sample remains
  // encrypted.  The monobit count measures balance: an unbiased source should
  // produce approximately half one bits across all 16,384 sampled bits.  The
  // transition count measures independence between consecutive draws: XORing
  // neighbors should likewise change approximately half of their bit positions.
  // Only these aggregate counters (and two example values) are decrypted after
  // sampling, so no random value can steer the analysis control flow.
  uint64e_t one_bits = 0;
  uint64e_t transitions = 0;
  uint64e_t previous = 0;
  for (unsigned i = 0; i < sample_count; ++i) {
    samples[i] = certified_random<0>();
    uint64e_t x = samples[i];
    for (unsigned bit = 0; bit < 64; ++bit)
      one_bits += (x >> bit) & 1;
    if (i != 0) {
      uint64e_t changed = x ^ previous;
      for (unsigned bit = 0; bit < 64; ++bit)
        transitions += (changed >> bit) & 1;
    }
    previous = x;
  }

  const uint64_t ones = one_bits.decrypt();
  const uint64_t flips = transitions.decrypt();
  const uint64_t total_bits = sample_count * 64u;
  const uint64_t total_pairs = (sample_count - 1u) * 64u;
  libmin_printf("CERTRNG samples=%u first=0x%016lx last=0x%016lx\n",
                sample_count, samples[0].decrypt(),
                samples[sample_count - 1].decrypt());
  libmin_printf("randomness: one-bits=%lu/%lu (%lu.%02lu%%), transitions=%lu/%lu (%lu.%02lu%%)\n",
                ones, total_bits,
                ones * 100 / total_bits,
                ones * 10000 / total_bits % 100,
                flips, total_pairs,
                flips * 100 / total_pairs,
                flips * 10000 / total_pairs % 100);
  libmin_printf("expected: one-bits about %lu/%lu (50.00%%), transitions about %lu/%lu (50.00%%)\n",
                total_bits / 2, total_bits, total_pairs / 2, total_pairs);
  libmin_success();
  return 0;
}
