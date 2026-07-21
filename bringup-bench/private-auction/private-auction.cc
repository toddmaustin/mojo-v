#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-proofcarrying.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_proofcarrying_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_proofcarrying_fp64_t
#include "mojov-exo.h"

#define AUCTION_BIDDERS 8u
#define BID_BRAND_BASE 0x41554354494f4e00ull
#define BIDDER_BRAND_BASE 0x4249444445520000ull

static uint128_t simon_key = SIMON128_KEY;
static simon_state_t simon_state;

static mojov_mem_proofcarrying_u64_t
encrypt_bid(uint64_t bid, uint64_t brand)
{
  const uint64_t dfhash = mojov_hash64(mojov_hash64_init(), brand);
  mojov_mem_proofcarrying_u64_t ptval;
  ptval.pt.val = bid;
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
    libmin_printf("ERROR: proof-carrying debug decrypt failed for auction value.\n");
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
  ptval.pt.metadata = 0x5052495641554354ull;
  mojov_mem_datagrant_t ctval;

  simon_128_128_encrypt(&simon_state, ptval.ct.ct_lo, &ctval.ct.ct_lo);
  simon_128_128_encrypt(&simon_state, (ptval.ct.ct_hi ^ ctval.ct.ct_lo), &ctval.ct.ct_hi);

  return ctval;
}

static void
compute_encrypted_winner(const mojov_mem_proofcarrying_u64_t encrypted_bids[AUCTION_BIDDERS],
                         uint64e_t *winning_bid_out,
                         uint64e_t *winning_bidder_out)
{
  uint64e_t winning_bid(encrypted_bids[0]);
  uint64e_t winning_bidder(encrypt_bid(0u, BIDDER_BRAND_BASE));

  for (unsigned i = 1; i < AUCTION_BIDDERS; i++)
  {
    uint64e_t candidate_bid(encrypted_bids[i]);
    uint64e_t candidate_id(encrypt_bid(i, BIDDER_BRAND_BASE + i));
    uint64e_t candidate_wins = candidate_bid > winning_bid;

    winning_bid = cmov(candidate_wins, candidate_bid, winning_bid);
    winning_bidder = cmov(candidate_wins, candidate_id, winning_bidder);
  }

  *winning_bid_out = winning_bid;
  *winning_bidder_out = winning_bidder;
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
  static const uint64_t raw_bids[AUCTION_BIDDERS] = {
    110u, 245u, 180u, 245u, 320u, 275u, 319u, 150u
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

  mojov_mem_proofcarrying_u64_t encrypted_bids[AUCTION_BIDDERS];
  for (unsigned i = 0; i < AUCTION_BIDDERS; i++)
    encrypted_bids[i] = encrypt_bid(raw_bids[i], BID_BRAND_BASE + i);

  uint64e_t expected_winning_bid;
  uint64e_t expected_winning_bidder;
  compute_encrypted_winner(encrypted_bids, &expected_winning_bid, &expected_winning_bidder);

  datagrant_t winning_bid_grant(make_datagrant(debug_dfhash_u64(expected_winning_bid.encrypted())));
  datagrant_t winning_bidder_grant(make_datagrant(debug_dfhash_u64(expected_winning_bidder.encrypted())));

  uint64e_t winning_bid;
  uint64e_t winning_bidder;
  compute_encrypted_winner(encrypted_bids, &winning_bid, &winning_bidder);

  const datagrant_t::plaintext_type winning_bid_grant_plaintext = winning_bid_grant.decrypt();
  const datagrant_t::plaintext_type winning_bidder_grant_plaintext = winning_bidder_grant.decrypt();
  check_dfhash_integrity("winning_bid",
                         winning_bid_grant_plaintext.dfhash,
                         debug_dfhash_u64(winning_bid.encrypted()));
  check_dfhash_integrity("winning_bidder",
                         winning_bidder_grant_plaintext.dfhash,
                         debug_dfhash_u64(winning_bidder.encrypted()));

  const uint16_t mojov_arg = read_mojov_arg();

  libmin_printf("Private auction safe disclosure demo\n");
  libmin_printf("  bidders: %u encrypted bids\n", AUCTION_BIDDERS);
  libmin_printf("  debug: winning bid grant valid: %s\n", winning_bid_grant.is_valid() ? "yes" : "no");
  libmin_printf("  debug: winning bidder grant valid: %s\n", winning_bidder_grant.is_valid() ? "yes" : "no");
  libmin_printf("INFO: Running private auction Mojo-V test %u...\n", (uint32_t)mojov_arg);

  switch (mojov_arg)
  {
  // Positive test: disclose both auction outputs with their matching datagrants.
  case 0:
  {
    (void)_testdatagrant(winning_bid.encrypted(), &winning_bid_grant.encrypted());
    (void)_testdatagrant(winning_bidder.encrypted(), &winning_bidder_grant.encrypted());

    const uint64_t disclosed_winning_bid = _disclose(winning_bid.encrypted(), &winning_bid_grant.encrypted());
    const uint64_t disclosed_winning_bidder = _disclose(winning_bidder.encrypted(), &winning_bidder_grant.encrypted());

    libmin_printf("  disclosed winner: bidder %lu\n", disclosed_winning_bidder);
    libmin_printf("  disclosed winning bid: %lu\n", disclosed_winning_bid);
    libmin_printf("SUCCESS: disclosed winning_bidder (4) and winning big (320).\n");
    libmin_printf("SUCCESS: raw losing bids remain encrypted; only granted aggregate results were disclosed.\n");

    if (disclosed_winning_bidder != 4u || disclosed_winning_bid != 320u)
    {
      libmin_printf("ERROR: private auction disclosed the wrong winner.\n");
      libmin_fail(-1);
    }
    break;
  }

  // Negative test: first validate winning_bid_grant against winning_bid, then
  // try to use that grant to disclose winning_bidder.
  case 1:
    libmin_printf("  negative test: disclose winning_bidder with winning_bid_grant.\n");
    (void)_testdatagrant(winning_bid.encrypted(), &winning_bid_grant.encrypted());
    (void)_disclose(winning_bidder.encrypted(), &winning_bid_grant.encrypted());
    negative_test_failed("winning bidder mismatched datagrant test");
    break;

  // Negative test: first validate winning_bid_grant against winning_bid, then
  // try to use that grant to disclose the raw encrypted bid from bidder 4.
  case 2:
  {
    libmin_printf("  negative test: disclose encrypted_bids[4] with winning_bid_grant.\n");
    (void)_testdatagrant(winning_bid.encrypted(), &winning_bid_grant.encrypted());
    (void)_disclose(encrypted_bids[4], &winning_bid_grant.encrypted());
    negative_test_failed("winning raw bid mismatched datagrant test");
    break;
  }

  // Negative test: first validate winning_bidder_grant against winning_bidder, then
  // try to use that grant to disclose winning_bid.
  case 3:
  {
    libmin_printf("  negative test: disclose winning_bid with winning_bidder_grant.\n");
    (void)_testdatagrant(winning_bidder.encrypted(), &winning_bidder_grant.encrypted());
    (void)_disclose(winning_bid.encrypted(), &winning_bidder_grant.encrypted());
    negative_test_failed("winning bid mismatched datagrant test");
    break;
  }

  // Negative test: use ciphertext that is not a datagrant. This should trap
  // when _testdatagrant() checks the bogus datagrant before disclosure.
  case 4:
  {
    libmin_printf("  negative test: disclose winning_bid with bogus datagrant ciphertext.\n");
    mojov_mem_datagrant_t *bogus_grant =
      (mojov_mem_datagrant_t *)&encrypted_bids[0];
    (void)_testdatagrant(winning_bid.encrypted(), bogus_grant);
    (void)_disclose(winning_bid.encrypted(), bogus_grant);
    negative_test_failed("bogus datagrant ciphertext test");
    break;
  }

  // Negative test: use a datagrant ciphertext with a valid datagrant encoding
  // but a tampered dfhash, then try to disclose winning_bid with it.
  case 5:
  {
    libmin_printf("  negative test: disclose winning_bid with tampered datagrant dfhash.\n");
    datagrant_t tampered_grant(make_datagrant(winning_bid_grant_plaintext.dfhash ^ 1u));
    (void)_testdatagrant(winning_bid.encrypted(), &tampered_grant.encrypted());
    (void)_disclose(winning_bid.encrypted(), &tampered_grant.encrypted());
    negative_test_failed("tampered datagrant dfhash test");
    break;
  }

  // Negative test: validate the original winning_bid_grant, then try to replay
  // it against a recomputed winning bid from a modified auction.
  case 6:
  {
    libmin_printf("  negative test: disclose modified auction winner with stale winning_bid_grant.\n");
    mojov_mem_proofcarrying_u64_t modified_bids[AUCTION_BIDDERS];
    for (unsigned i = 0; i < AUCTION_BIDDERS; i++)
      modified_bids[i] = encrypted_bids[i];
    modified_bids[7] = encrypt_bid(500u, BID_BRAND_BASE + /*do not reuse brand */107u);

    uint64e_t modified_winning_bid;
    uint64e_t modified_winning_bidder;
    compute_encrypted_winner(modified_bids, &modified_winning_bid, &modified_winning_bidder);

    (void)_testdatagrant(winning_bid.encrypted(), &winning_bid_grant.encrypted());
    (void)_disclose(modified_winning_bid.encrypted(), &winning_bid_grant.encrypted());
    negative_test_failed("stale winning bid datagrant test");
    break;
  }

  // Negative test: compute winning_bid + encrypted_one, then try to
  // disclose that derived value with the grant for the original winning_bid.
  case 7:
  {
    libmin_printf("  negative test: disclose winning_bid plus encrypted_one with winning_bid_grant.\n");
    uint64e_t encrypted_one(encrypt_bid(1u, BID_BRAND_BASE + 0x100u));
    uint64e_t derived_winning_bid = winning_bid + encrypted_one;
    (void)_testdatagrant(winning_bid.encrypted(), &winning_bid_grant.encrypted());
    (void)_disclose(derived_winning_bid.encrypted(), &winning_bid_grant.encrypted());
    negative_test_failed("encrypted-one derived winning bid datagrant test");
    break;
  }

  // Negative test: compute winning_bid + 1 with an immediate operand, then try
  // to disclose that derived value with the grant for the original winning_bid.
  case 8:
  {
    libmin_printf("  negative test: disclose winning_bid plus immediate one with winning_bid_grant.\n");
    uint64e_t incremented_winning_bid = winning_bid + 1u;
    (void)_testdatagrant(winning_bid.encrypted(), &winning_bid_grant.encrypted());
    (void)_disclose(incremented_winning_bid.encrypted(), &winning_bid_grant.encrypted());
    negative_test_failed("immediate-one derived winning bid datagrant test");
    break;
  }

  // Negative test: compute an intermediate comparison predicate and try to
  // disclose that predicate with the winning_bid_grant.
  case 9:
  {
    libmin_printf("  negative test: disclose intermediate predicate with winning_bid_grant.\n");
    uint64e_t bidder4_bid(encrypted_bids[4]);
    uint64e_t bidder0_bid(encrypted_bids[0]);
    uint64e_t bidder4_beats_bidder0 = bidder4_bid > bidder0_bid;
    (void)_testdatagrant(winning_bid.encrypted(), &winning_bid_grant.encrypted());
    (void)_disclose(bidder4_beats_bidder0.encrypted(), &winning_bid_grant.encrypted());
    negative_test_failed("intermediate predicate datagrant test");
    break;
  }

  default:
    libmin_printf("ERROR: invalid private auction test (%u).\n", (uint32_t)mojov_arg);
    libmin_fail(-1);
  }

  libmin_success();
  return 0;
}
