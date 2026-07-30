#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"
#include "dc-proofcarrying.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_proofcarrying_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_proofcarrying_fp64_t
#include "mojov-exo.h"

using namespace exo;

#define LOTTO_NONCE_BRAND 0x4c4f54544f4e4f4eull
#define STALE_NONCE_BRAND 0x4f4c444c4f54544full
#define RESULT_GRANT_BRAND 0x4c4f54544f47524eull
#define DESIRED_WINNER 4u
#define PARTICIPANTS 8u

static uint128_t simon_key = SIMON128_KEY;
static simon_state_t simon_state;

static mojov_mem_proofcarrying_u64_t encrypt_value(uint64_t value, uint64_t brand)
{
  mojov_mem_proofcarrying_u64_t plaintext;
  plaintext.pt.val = value;
  plaintext.pt.salt = ((uint64_t)libmin_rand() << 32) | libmin_rand();
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
  plaintext.pt.salt = ((uint64_t)libmin_rand() << 32) | libmin_rand();
  plaintext.pt.sig = CONTRACT_SIG + 1u;
  plaintext.pt.metadata = RESULT_GRANT_BRAND;
  mojov_mem_datagrant_t ciphertext;
  simon_128_128_encrypt(&simon_state, plaintext.ct.ct_lo, &ciphertext.ct.ct_lo);
  simon_128_128_encrypt(&simon_state, plaintext.ct.ct_hi ^ ciphertext.ct.ct_lo,
                        &ciphertext.ct.ct_hi);
  return ciphertext;
}

/* A fixed, data-oblivious lexicographic tournament. Higher score wins; equal
 * scores are resolved by the lower certified priority. Every iteration feeds
 * its score, priority, and candidate ID into the returned value's receipt. */
static uint64e_t select_winner(uint64e_t scores[PARTICIPANTS],
                               uint64e_t priorities[PARTICIPANTS])
{
  uint64e_t best_score = scores[0];
  uint64e_t best_priority = priorities[0];
  uint64e_t best_id((uint64_t)0);
  for (unsigned i = 1; i < PARTICIPANTS; ++i) {
    uint64e_t higher = best_score < scores[i];
    uint64e_t tied = best_score == scores[i];
    uint64e_t take = higher || (tied && (priorities[i] < best_priority));
    best_score = cmov(take, scores[i], best_score);
    best_priority = cmov(take, priorities[i], best_priority);
    best_id = cmov(take, (uint64_t)i, best_id);
  }
  return best_id;
}

static void approved_priorities(uint64e_t nonce,
                                uint64e_t priorities[PARTICIPANTS])
{
  priorities[0] = certified_random<0>() ^ nonce;
  priorities[1] = certified_random<1>() ^ nonce;
  priorities[2] = certified_random<2>() ^ nonce;
  priorities[3] = certified_random<3>() ^ nonce;
  priorities[4] = certified_random<4>() ^ nonce;
  priorities[5] = certified_random<5>() ^ nonce;
  priorities[6] = certified_random<6>() ^ nonce;
  priorities[7] = certified_random<7>() ^ nonce;
}

static uint64e_t approved_lottery(uint64e_t scores[PARTICIPANTS], uint64e_t nonce)
{
  uint64e_t priorities[PARTICIPANTS];
  approved_priorities(nonce, priorities);
  return select_winner(scores, priorities);
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
  const uint64_t clear_scores[PARTICIPANTS] = {12, 19, 19, 7, 19, 15, 15, 3};
  uint64e_t scores[PARTICIPANTS];
  for (unsigned i = 0; i < PARTICIPANTS; ++i)
    scores[i] = uint64e_t(encrypt_value(clear_scores[i], 0x53434f5245000000ull + i));

  uint64e_t nonce(encrypt_value(0xd1b54a32d192ed03ull, LOTTO_NONCE_BRAND));
  uint64e_t stale_nonce(encrypt_value(0x94d049bb133111ebull, STALE_NONCE_BRAND));
  uint64e_t expected = approved_lottery(scores, nonce);
  const uint64_t expected_hash = debug_dfhash(expected.encrypted());
  datagrant_t grant(make_datagrant(expected_hash));
  const unsigned test = read_mojov_arg();

  libmin_printf("Certified Lottery (8 encrypted participants; ties 1/2/4 and 5/6)\n");
  libmin_printf("INFO: Running certified lotto Mojo-V test %u...\n", test);

  uint64e_t selected;
  switch (test) {
  case 0:
    libmin_printf("  positive test: honest certified tied draw.\n");
    selected = approved_lottery(scores, nonce);
    break;
  case 1:
    libmin_printf("  positive control: unrelated unused certified draw.\n");
    (void)certified_random<100>();
    selected = approved_lottery(scores, nonce);
    break;
  case 2: {
    libmin_printf("  positive unsafe control: disclose-before-commit grinding.\n");
    unsigned trials = 0;
    uint64_t disclosed = 0;
    do {
      uint64e_t attempt = approved_lottery(scores, nonce);
      disclosed = _disclose(attempt.encrypted(), &grant.encrypted());
      ++trials;
    } while (disclosed != DESIRED_WINNER && trials < 50);
    libmin_printf("Committed result:          NO (disclosed first)\n");
    libmin_printf("Selected participant:      server-visible %lu\n", disclosed);
    libmin_printf("Grinding target reached:   %s after %u trials\n",
                  disclosed == DESIRED_WINNER ? "YES" : "NO", trials);
    libmin_printf("TAKEAWAY: commit the ciphertext and receipt before disclosure.\n");
    libmin_success();
    return 0;
  }
  case 10: {
    libmin_printf("  negative test: omit eligible participant 4.\n");
    uint64e_t altered[PARTICIPANTS];
    for (unsigned i = 0; i < PARTICIPANTS; ++i) altered[i] = scores[i];
    altered[4] = scores[1];
    selected = approved_lottery(altered, nonce);
    break;
  }
  case 11: {
    libmin_printf("  negative test: software RNG replacement.\n");
    uint64e_t priorities[PARTICIPANTS];
    approved_priorities(nonce, priorities);
    priorities[2] = uint64e_t(((uint64_t)libmin_rand() << 32) | libmin_rand()) ^ nonce;
    selected = select_winner(scores, priorities);
    break;
  }
  case 12: {
    libmin_printf("  negative test: skip request nonce.\n");
    uint64e_t priorities[PARTICIPANTS];
    approved_priorities(nonce, priorities);
    priorities[1] = certified_random<1>();
    selected = select_winner(scores, priorities);
    break;
  }
  case 13:
    libmin_printf("  negative test: replay stale request.\n");
    selected = approved_lottery(scores, stale_nonce);
    break;
  case 14: {
    libmin_printf("  negative test: reuse one draw.\n");
    uint64e_t one = certified_random<0>() ^ nonce;
    uint64e_t priorities[PARTICIPANTS] = {one, one, one, one, one, one, one, one};
    selected = select_winner(scores, priorities);
    break;
  }
  case 15: {
    libmin_printf("  negative test: swap participant draw sites.\n");
    uint64e_t priorities[PARTICIPANTS];
    approved_priorities(nonce, priorities);
    priorities[1] = certified_random<4>() ^ nonce;
    priorities[4] = certified_random<1>() ^ nonce;
    selected = select_winner(scores, priorities);
    break;
  }
  case 16: {
    libmin_printf("  negative test: deterministic tie-break.\n");
    uint64e_t priorities[PARTICIPANTS] = {
      uint64e_t(0), uint64e_t(0), uint64e_t(0), uint64e_t(0),
      uint64e_t(0), uint64e_t(0), uint64e_t(0), uint64e_t(0)
    };
    selected = select_winner(scores, priorities);
    break;
  }
  default:
    libmin_printf("ERROR: invalid certified lotto test.\n");
    libmin_fail(-1);
  }

  const uint64_t returned_hash = debug_dfhash(selected.encrypted());
  libmin_printf("Selected participant:      encrypted (server cannot observe)\n");
  print_hash("Expected dfhash:           ", expected_hash);
  print_hash("Returned dfhash:           ", returned_hash);
  libmin_printf("Proof verification:        %s\n",
                returned_hash == expected_hash ? "PASS" : "FAIL");

  /* The ciphertext and its receipt become the committed result before any
   * client-side decryption. _testdatagrant validates without server disclosure. */
  (void)_testdatagrant(selected.encrypted(), &grant.encrypted());
  const uint64_t decrypt_hash = debug_dfhash(selected.encrypted());
  if (decrypt_hash != expected_hash) {
    libmin_printf("ERROR: committed winner receipt changed before decryption.\n");
    libmin_fail(-1);
  }
  libmin_printf("Committed result:          YES (ciphertext plus accepted receipt)\n");
  const uint64_t winner = selected.decrypt();
  libmin_printf("Selected participant:      client-decrypted %lu\n", winner);
  libmin_printf("SUCCESS: every eligible participant and certified draw accepted.\n");
  libmin_success();
  return 0;
}
