#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"
#include "dc-proofcarrying.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_proofcarrying_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_proofcarrying_fp64_t
#include "mojov-exo.h"

using namespace exo;

#define QUERY_BRAND 0x4450434f554e5451ull
#define STALE_QUERY_BRAND 0x4f4c444450515259ull
#define TRUE_COUNT 23u

static uint128_t simon_key = SIMON128_KEY;
static simon_state_t simon_state;

static mojov_mem_proofcarrying_u64_t encrypt_value(uint64_t value, uint64_t brand)
{
  mojov_mem_proofcarrying_u64_t plaintext;
  plaintext.pt.val = value;
  plaintext.pt.salt = (((uint64_t)libmin_rand()) << 32) | libmin_rand();
  plaintext.pt.sig = CONTRACT_SIG;
  plaintext.pt.metadata = mojov_hash64(mojov_hash64_init(), brand);
  mojov_mem_proofcarrying_u64_t ciphertext;
  simon_128_128_encrypt(&simon_state, plaintext.ct.ct_lo, &ciphertext.ct.ct_lo);
  simon_128_128_encrypt(&simon_state, plaintext.ct.ct_hi ^ ciphertext.ct.ct_lo,
                        &ciphertext.ct.ct_hi);
  return ciphertext;
}

static uint64_t debug_dfhash(mojov_mem_proofcarrying_u64_t value)
{
  mojov_mem_proofcarrying_u64_t plaintext;
  simon_128_128_decrypt(&simon_state, value.ct.ct_lo, &plaintext.ct.ct_lo);
  simon_128_128_decrypt(&simon_state, value.ct.ct_hi, &plaintext.ct.ct_hi);
  plaintext.ct.ct_hi ^= value.ct.ct_lo;
  return plaintext.pt.metadata;
}

static mojov_mem_datagrant_t make_datagrant(uint64_t dfhash)
{
  mojov_mem_datagrant_t plaintext;
  plaintext.pt.dfhash = dfhash;
  plaintext.pt.salt = (((uint64_t)libmin_rand()) << 32) | libmin_rand();
  plaintext.pt.sig = CONTRACT_SIG + 1u;
  plaintext.pt.metadata = 0x4450434f554e5447ull;
  mojov_mem_datagrant_t ciphertext;
  simon_128_128_encrypt(&simon_state, plaintext.ct.ct_lo, &ciphertext.ct.ct_lo);
  simon_128_128_encrypt(&simon_state, plaintext.ct.ct_hi ^ ciphertext.ct.ct_lo,
                        &ciphertext.ct.ct_hi);
  return ciphertext;
}

/* Centered binomial noise: Binomial(8, 1/2) - Binomial(8, 1/2).  The query
 * nonce is mixed into every bit, binding all 16 certified draws to one request.
 * TRUE_COUNT is larger than the maximum magnitude, so unsigned arithmetic is
 * sufficient and the released answer lies in [15, 31]. */
static uint64e_t approved_private_count(uint64e_t query)
{
  uint64e_t answer = query;
  answer = answer + ((certified_random<0>() ^ query) & (uint64_t)1);
  answer = answer + ((certified_random<1>() ^ query) & (uint64_t)1);
  answer = answer + ((certified_random<2>() ^ query) & (uint64_t)1);
  answer = answer + ((certified_random<3>() ^ query) & (uint64_t)1);
  answer = answer + ((certified_random<4>() ^ query) & (uint64_t)1);
  answer = answer + ((certified_random<5>() ^ query) & (uint64_t)1);
  answer = answer + ((certified_random<6>() ^ query) & (uint64_t)1);
  answer = answer + ((certified_random<7>() ^ query) & (uint64_t)1);
  answer = answer - ((certified_random<8>() ^ query) & (uint64_t)1);
  answer = answer - ((certified_random<9>() ^ query) & (uint64_t)1);
  answer = answer - ((certified_random<10>() ^ query) & (uint64_t)1);
  answer = answer - ((certified_random<11>() ^ query) & (uint64_t)1);
  answer = answer - ((certified_random<12>() ^ query) & (uint64_t)1);
  answer = answer - ((certified_random<13>() ^ query) & (uint64_t)1);
  answer = answer - ((certified_random<14>() ^ query) & (uint64_t)1);
  answer = answer - ((certified_random<15>() ^ query) & (uint64_t)1);
  return answer;
}

static uint16_t read_mojov_arg()
{
  return (uint16_t)((mojov_read_mprivregcfg() >> 12) & 0xffffu);
}

static void print_hash(const char *label, uint64_t hash)
{
  libmin_printf("%s0x%08x%08x\n", label, (uint32_t)(hash >> 32), (uint32_t)hash);
}

int main(void)
{
  if (mojov_configure_kmsm_from_dc_proof() != 0 ||
      mojov_enable_and_verify() != 0 ||
      !simon_128_128_keyexpand(&simon_state, simon_key, 68) ||
      debug_context(SIMON128_KEY, CONTRACT_SIG) != 0)
    return -1;

  libmin_srand(42);
  uint64e_t query(encrypt_value(TRUE_COUNT, QUERY_BRAND));
  uint64e_t stale_query(encrypt_value(TRUE_COUNT, STALE_QUERY_BRAND));
  uint64e_t expected = approved_private_count(query);
  const uint64_t expected_hash = debug_dfhash(expected.encrypted());
  datagrant_t grant(make_datagrant(expected_hash));
  const unsigned test = read_mojov_arg();

  libmin_printf("Verifiable Differentially Private Count (centered binomial noise)\n");
  libmin_printf("INFO: Running DP count Mojo-V test %u...\n", test);

  uint64e_t released;
  switch (test) {
  case 0:
    libmin_printf("  positive test: approved certified noise mechanism.\n");
    released = approved_private_count(query);
    break;
  case 1:
    libmin_printf("  positive control: unrelated unused certified draw.\n");
    (void)certified_random<100>();
    released = approved_private_count(query);
    break;
  case 10:
    libmin_printf("  negative test: omit noise.\n");
    released = query;
    break;
  case 11: {
    libmin_printf("  negative test: reduce noise magnitude.\n");
    released = query;
    released = released + ((certified_random<0>() ^ query) & (uint64_t)1);
    released = released + ((certified_random<1>() ^ query) & (uint64_t)1);
    released = released + ((certified_random<2>() ^ query) & (uint64_t)1);
    released = released + ((certified_random<3>() ^ query) & (uint64_t)1);
    released = released - ((certified_random<8>() ^ query) & (uint64_t)1);
    released = released - ((certified_random<9>() ^ query) & (uint64_t)1);
    released = released - ((certified_random<10>() ^ query) & (uint64_t)1);
    released = released - ((certified_random<11>() ^ query) & (uint64_t)1);
    break;
  }
  case 12: {
    libmin_printf("  negative test: software RNG replacement.\n");
    released = query;
    for (unsigned i = 0; i < 8; ++i)
      released = released + (uint64_t)(libmin_rand() & 1u);
    for (unsigned i = 0; i < 8; ++i)
      released = released - (uint64_t)(libmin_rand() & 1u);
    break;
  }
  case 13: {
    /* A second valid sample is computed, then the server keeps the larger one.
     * This models favorable resampling without revealing either plaintext. */
    libmin_printf("  negative test: favorable resampling.\n");
    uint64e_t first = approved_private_count(query);
    uint64e_t second = query;
    second = second + ((certified_random<20>() ^ query) & (uint64_t)1);
    second = second + ((certified_random<21>() ^ query) & (uint64_t)1);
    second = second + ((certified_random<22>() ^ query) & (uint64_t)1);
    second = second + ((certified_random<23>() ^ query) & (uint64_t)1);
    second = second + ((certified_random<24>() ^ query) & (uint64_t)1);
    second = second + ((certified_random<25>() ^ query) & (uint64_t)1);
    second = second + ((certified_random<26>() ^ query) & (uint64_t)1);
    second = second + ((certified_random<27>() ^ query) & (uint64_t)1);
    second = second - ((certified_random<28>() ^ query) & (uint64_t)1);
    second = second - ((certified_random<29>() ^ query) & (uint64_t)1);
    second = second - ((certified_random<30>() ^ query) & (uint64_t)1);
    second = second - ((certified_random<31>() ^ query) & (uint64_t)1);
    second = second - ((certified_random<32>() ^ query) & (uint64_t)1);
    second = second - ((certified_random<33>() ^ query) & (uint64_t)1);
    second = second - ((certified_random<34>() ^ query) & (uint64_t)1);
    second = second - ((certified_random<35>() ^ query) & (uint64_t)1);
    released = cmov(first < second, second, first);
    break;
  }
  case 14:
    libmin_printf("  negative test: replay old query.\n");
    released = approved_private_count(stale_query);
    break;
  default:
    libmin_printf("ERROR: invalid DP count test.\n");
    libmin_fail(-1);
  }

  const uint64_t returned_hash = debug_dfhash(released.encrypted());
  libmin_printf("Released count:            encrypted (server cannot observe)\n");
  print_hash("Expected dfhash:           ", expected_hash);
  print_hash("Returned dfhash:           ", returned_hash);
  libmin_printf("Proof verification:        %s\n",
                returned_hash == expected_hash ? "PASS" : "FAIL");

  (void)_testdatagrant(released.encrypted(), &grant.encrypted());
  const uint64_t decrypt_dfhash = debug_dfhash(released.encrypted());
  if (decrypt_dfhash != expected_hash) {
    libmin_printf("ERROR: private-count dfhash changed before client decryption.\n");
    libmin_printf("DEBUG: expected dfhash 0x%08x%08x, but received 0x%08x%08x\n",
                  (uint32_t)(expected_hash >> 32), (uint32_t)expected_hash,
                  (uint32_t)(decrypt_dfhash >> 32), (uint32_t)decrypt_dfhash);
    libmin_fail(-1);
  }
  const uint64_t decrypted = released.decrypt();
  libmin_printf("Released count:            client-decrypted value %lu\n", decrypted);
  libmin_printf("SUCCESS: exact request-bound certified noise graph accepted.\n");
  libmin_success();
  return 0;
}
