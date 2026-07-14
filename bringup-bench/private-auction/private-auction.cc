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

  libmin_printf("Private auction safe disclosure demo\n");
  libmin_printf("  bidders: %u encrypted bids\n", AUCTION_BIDDERS);
  libmin_printf("  debug: winning bid grant valid: %s\n", winning_bid_grant.is_valid() ? "yes" : "no");
  libmin_printf("  debug: winning bidder grant valid: %s\n", winning_bidder_grant.is_valid() ? "yes" : "no");

  libmin_printf("    _loaddatagrant...\n");

  (void)_loaddatagrant(&winning_bid_grant.encrypted());
  libmin_success();

  libmin_printf("    _testdatagrant...\n");
  (void)_testdatagrant(winning_bid.encrypted(), &winning_bid_grant.encrypted());
  libmin_printf("    _testdatagrant...\n");
  (void)_testdatagrant(winning_bidder.encrypted(), &winning_bidder_grant.encrypted());

  const uint64_t disclosed_winning_bid = _disclose(winning_bid.encrypted(), &winning_bid_grant.encrypted());
  const uint64_t disclosed_winning_bidder = _disclose(winning_bidder.encrypted(), &winning_bidder_grant.encrypted());

  libmin_printf("  disclosed winner: bidder %lu\n", disclosed_winning_bidder);
  libmin_printf("  disclosed winning bid: %lu\n", disclosed_winning_bid);
  libmin_printf("  negative path: disclosing with a mismatched datagrant would trap.\n");
  libmin_printf("  raw losing bids remain encrypted; only granted aggregate results were disclosed.\n");

  if (disclosed_winning_bidder != 4u || disclosed_winning_bid != 320u)
  {
    libmin_printf("ERROR: private auction disclosed the wrong winner.\n");
    libmin_fail(-1);
  }

  libmin_success();
  return 0;
}
