#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-proofcarrying.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_proofcarrying_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_proofcarrying_fp64_t
#include "mojov-exo.h"

#define VOTE_BALLOTS 12u
#define VOTE_CANDIDATES 3u
#define BALLOT_BRAND_BASE 0x42414c4c4f540000ull
#define TALLY_BRAND_BASE 0x54414c4c59000000ull

static uint128_t simon_key = SIMON128_KEY;
static simon_state_t simon_state;

static mojov_mem_proofcarrying_u64_t
encrypt_vote_value(uint64_t value, uint64_t brand)
{
  const uint64_t dfhash = mojov_hash64(mojov_hash64_init(), brand);
  mojov_mem_proofcarrying_u64_t ptval;
  ptval.pt.val = value;
  ptval.pt.salt = (((uint64_t)libmin_rand()) << 32) | (uint64_t)libmin_rand();
  ptval.pt.sig = CONTRACT_SIG;
  ptval.pt.metadata = dfhash;
  mojov_mem_proofcarrying_u64_t ctval;

  simon_128_128_encrypt(&simon_state, ptval.ct.ct_lo, &ctval.ct.ct_lo);
  simon_128_128_encrypt(&simon_state, (ptval.ct.ct_hi ^ ctval.ct.ct_lo), &ctval.ct.ct_hi);

  return ctval;
}

static uint64_t
debug_dfhash_u64(mojov_mem_proofcarrying_u64_t value)
{
  mojov_mem_proofcarrying_u64_t plaintext;
  simon_128_128_decrypt(&simon_state, value.ct.ct_lo, &plaintext.ct.ct_lo);
  simon_128_128_decrypt(&simon_state, value.ct.ct_hi, &plaintext.ct.ct_hi);
  plaintext.ct.ct_hi ^= value.ct.ct_lo;

  if (plaintext.pt.sig != CONTRACT_SIG)
  {
    libmin_printf("ERROR: proof-carrying debug decrypt failed for vote tally value.\n");
    libmin_fail(-1);
  }

  return plaintext.pt.metadata;
}

static mojov_mem_datagrant_t
make_datagrant(uint64_t dfhash)
{
  mojov_mem_datagrant_t ptval;
  ptval.pt.dfhash = dfhash;
  ptval.pt.salt = (((uint64_t)libmin_rand()) << 32) | (uint64_t)libmin_rand();
  ptval.pt.sig = CONTRACT_SIG + 1u;
  ptval.pt.metadata = 0x564f544547524e54ull;
  mojov_mem_datagrant_t ctval;

  simon_128_128_encrypt(&simon_state, ptval.ct.ct_lo, &ctval.ct.ct_lo);
  simon_128_128_encrypt(&simon_state, (ptval.ct.ct_hi ^ ctval.ct.ct_lo), &ctval.ct.ct_hi);

  return ctval;
}

static void
compute_encrypted_tallies(const mojov_mem_proofcarrying_u64_t encrypted_ballots[VOTE_BALLOTS],
                          uint64e_t tallies[VOTE_CANDIDATES])
{
  tallies[0] = uint64e_t(encrypt_vote_value(0u, TALLY_BRAND_BASE + 0u));
  tallies[1] = uint64e_t(encrypt_vote_value(0u, TALLY_BRAND_BASE + 1u));
  tallies[2] = uint64e_t(encrypt_vote_value(0u, TALLY_BRAND_BASE + 2u));

  for (unsigned i = 0; i < VOTE_BALLOTS; i++)
  {
    uint64e_t ballot(encrypted_ballots[i]);
    tallies[0] = tallies[0] + (ballot == 0u);
    tallies[1] = tallies[1] + (ballot == 1u);
    tallies[2] = tallies[2] + (ballot == 2u);
  }
}

static uint16_t
read_mojov_arg(void)
{
  return (uint16_t)((mojov_read_mprivregcfg() >> 12) & 0xffffu);
}

static void
check_dfhash_integrity(const char *name, uint64_t expected_dfhash, uint64_t received_dfhash)
{
  if (expected_dfhash != received_dfhash)
  {
    libmin_printf("ERROR: Computational integrity error on computation of %s\n", name);
    libmin_printf("DEBUG: expected dfhash 0x%08x%08x, but received 0x%08x%08x\n",
                  (uint32_t)(expected_dfhash >> 32), (uint32_t)expected_dfhash,
                  (uint32_t)(received_dfhash >> 32), (uint32_t)received_dfhash);
    libmin_fail(-1);
  }
}

static void
negative_test_failed(const char *test_name)
{
  libmin_printf("ERROR: %s failed because NO exception occurred!\n", test_name);
  libmin_fail(2);
}

int
main(void)
{
  static const uint64_t raw_ballots[VOTE_BALLOTS] = {
    0u, 1u, 2u, 1u, 0u, 2u, 1u, 1u, 0u, 2u, 1u, 0u
  };

  if (mojov_configure_kmsm_from_dc_proof() != 0)
    return -1;
  if (mojov_enable_and_verify() != 0)
    return -1;
  if (!simon_128_128_keyexpand(&simon_state, simon_key, 68))
    return -1;
  if (debug_context(SIMON128_KEY, CONTRACT_SIG) != 0)
    return -1;

  libmin_srand(42);

  mojov_mem_proofcarrying_u64_t encrypted_ballots[VOTE_BALLOTS];
  for (unsigned i = 0; i < VOTE_BALLOTS; i++)
    encrypted_ballots[i] = encrypt_vote_value(raw_ballots[i], BALLOT_BRAND_BASE + i);

  uint64e_t expected_tallies[VOTE_CANDIDATES];
  compute_encrypted_tallies(encrypted_ballots, expected_tallies);

  datagrant_t tally_a_grant(make_datagrant(debug_dfhash_u64(expected_tallies[0].encrypted())));
  datagrant_t tally_b_grant(make_datagrant(debug_dfhash_u64(expected_tallies[1].encrypted())));
  datagrant_t tally_c_grant(make_datagrant(debug_dfhash_u64(expected_tallies[2].encrypted())));

  uint64e_t tallies[VOTE_CANDIDATES];
  compute_encrypted_tallies(encrypted_ballots, tallies);

  const datagrant_t::plaintext_type tally_a_grant_plaintext = tally_a_grant.decrypt();
  const datagrant_t::plaintext_type tally_b_grant_plaintext = tally_b_grant.decrypt();
  const datagrant_t::plaintext_type tally_c_grant_plaintext = tally_c_grant.decrypt();
  check_dfhash_integrity("tally_A", tally_a_grant_plaintext.dfhash, debug_dfhash_u64(tallies[0].encrypted()));
  check_dfhash_integrity("tally_B", tally_b_grant_plaintext.dfhash, debug_dfhash_u64(tallies[1].encrypted()));
  check_dfhash_integrity("tally_C", tally_c_grant_plaintext.dfhash, debug_dfhash_u64(tallies[2].encrypted()));

  const uint16_t mojov_arg = read_mojov_arg();

  libmin_printf("Vote tally safe disclosure demo\n");
  libmin_printf("  ballots: %u encrypted votes over %u candidates\n", VOTE_BALLOTS, VOTE_CANDIDATES);
  libmin_printf("  debug: tally_A grant valid: %s\n", tally_a_grant.is_valid() ? "yes" : "no");
  libmin_printf("  debug: tally_B grant valid: %s\n", tally_b_grant.is_valid() ? "yes" : "no");
  libmin_printf("  debug: tally_C grant valid: %s\n", tally_c_grant.is_valid() ? "yes" : "no");
  libmin_printf("INFO: Running vote tally Mojo-V test %u...\n", (uint32_t)mojov_arg);

  switch (mojov_arg)
  {
  case 0:
  {
    (void)_testdatagrant(tallies[0].encrypted(), &tally_a_grant.encrypted());
    (void)_testdatagrant(tallies[1].encrypted(), &tally_b_grant.encrypted());
    (void)_testdatagrant(tallies[2].encrypted(), &tally_c_grant.encrypted());

    const uint64_t disclosed_tally_a = _disclose(tallies[0].encrypted(), &tally_a_grant.encrypted());
    const uint64_t disclosed_tally_b = _disclose(tallies[1].encrypted(), &tally_b_grant.encrypted());
    const uint64_t disclosed_tally_c = _disclose(tallies[2].encrypted(), &tally_c_grant.encrypted());

    libmin_printf("  disclosed tally_A: %lu\n", disclosed_tally_a);
    libmin_printf("  disclosed tally_B: %lu\n", disclosed_tally_b);
    libmin_printf("  disclosed tally_C: %lu\n", disclosed_tally_c);
    libmin_printf("SUCCESS: disclosed tally_A (4), tally_B (5), and tally_C (3).\n");
    libmin_printf("SUCCESS: individual ballots remain encrypted; only final aggregate counts were disclosed.\n");

    if (disclosed_tally_a != 4u || disclosed_tally_b != 5u || disclosed_tally_c != 3u)
    {
      libmin_printf("ERROR: vote tally disclosed the wrong aggregate counts.\n");
      libmin_fail(-1);
    }
    break;
  }

  case 1:
    libmin_printf("  negative test: disclose tally_B with tally_A_grant.\n");
    (void)_testdatagrant(tallies[0].encrypted(), &tally_a_grant.encrypted());
    (void)_disclose(tallies[1].encrypted(), &tally_a_grant.encrypted());
    negative_test_failed("tally_B mismatched datagrant test");
    break;

  case 2:
    libmin_printf("  negative test: disclose encrypted_ballots[0] with tally_A_grant.\n");
    (void)_testdatagrant(tallies[0].encrypted(), &tally_a_grant.encrypted());
    (void)_disclose(encrypted_ballots[0], &tally_a_grant.encrypted());
    negative_test_failed("raw ballot mismatched datagrant test");
    break;

  case 3:
    libmin_printf("  negative test: disclose tally_A with tally_B_grant.\n");
    (void)_testdatagrant(tallies[1].encrypted(), &tally_b_grant.encrypted());
    (void)_disclose(tallies[0].encrypted(), &tally_b_grant.encrypted());
    negative_test_failed("tally_A mismatched datagrant test");
    break;

  case 4:
  {
    libmin_printf("  negative test: disclose tally_A with bogus datagrant ciphertext.\n");
    mojov_mem_datagrant_t *bogus_grant = (mojov_mem_datagrant_t *)&encrypted_ballots[0];
    (void)_testdatagrant(tallies[0].encrypted(), bogus_grant);
    (void)_disclose(tallies[0].encrypted(), bogus_grant);
    negative_test_failed("bogus datagrant ciphertext test");
    break;
  }

  case 5:
  {
    libmin_printf("  negative test: disclose tally_A with tampered datagrant dfhash.\n");
    datagrant_t tampered_grant(make_datagrant(tally_a_grant_plaintext.dfhash ^ 1u));
    (void)_testdatagrant(tallies[0].encrypted(), &tampered_grant.encrypted());
    (void)_disclose(tallies[0].encrypted(), &tampered_grant.encrypted());
    negative_test_failed("tampered datagrant dfhash test");
    break;
  }

  case 6:
  {
    libmin_printf("  negative test: disclose modified election tally with stale tally_A_grant.\n");
    mojov_mem_proofcarrying_u64_t modified_ballots[VOTE_BALLOTS];
    for (unsigned i = 0; i < VOTE_BALLOTS; i++)
      modified_ballots[i] = encrypted_ballots[i];
    modified_ballots[11] = encrypt_vote_value(1u, BALLOT_BRAND_BASE + 111u);

    uint64e_t modified_tallies[VOTE_CANDIDATES];
    compute_encrypted_tallies(modified_ballots, modified_tallies);
    (void)_testdatagrant(tallies[0].encrypted(), &tally_a_grant.encrypted());
    (void)_disclose(modified_tallies[0].encrypted(), &tally_a_grant.encrypted());
    negative_test_failed("stale tally datagrant test");
    break;
  }

  case 7:
  {
    libmin_printf("  negative test: disclose tally_A plus encrypted_one with tally_A_grant.\n");
    uint64e_t encrypted_one(encrypt_vote_value(1u, TALLY_BRAND_BASE + 0x100u));
    uint64e_t derived_tally = tallies[0] + encrypted_one;
    (void)_testdatagrant(tallies[0].encrypted(), &tally_a_grant.encrypted());
    (void)_disclose(derived_tally.encrypted(), &tally_a_grant.encrypted());
    negative_test_failed("encrypted-one derived tally datagrant test");
    break;
  }

  case 8:
  {
    libmin_printf("  negative test: disclose tally_A plus immediate one with tally_A_grant.\n");
    uint64e_t incremented_tally = tallies[0] + 1u;
    (void)_testdatagrant(tallies[0].encrypted(), &tally_a_grant.encrypted());
    (void)_disclose(incremented_tally.encrypted(), &tally_a_grant.encrypted());
    negative_test_failed("immediate-one derived tally datagrant test");
    break;
  }

  case 9:
  {
    libmin_printf("  negative test: disclose intermediate ballot predicate with tally_A_grant.\n");
    uint64e_t first_ballot(encrypted_ballots[0]);
    uint64e_t first_ballot_is_a = first_ballot == 0u;
    (void)_testdatagrant(tallies[0].encrypted(), &tally_a_grant.encrypted());
    (void)_disclose(first_ballot_is_a.encrypted(), &tally_a_grant.encrypted());
    negative_test_failed("intermediate ballot predicate datagrant test");
    break;
  }

  default:
    libmin_printf("ERROR: invalid vote tally test (%u).\n", (uint32_t)mojov_arg);
    libmin_fail(-1);
  }

  libmin_success();
  return 0;
}
