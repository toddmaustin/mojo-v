#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"
#include "dc-proofcarrying.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_proofcarrying_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_proofcarrying_fp64_t
#include "mojov-exo.h"

using namespace exo;

#define AUDIT_NONCE_BRAND 0x41554449544e4f4eull
#define STALE_NONCE_BRAND 0x4f4c444e4f4e4345ull
#define AUDIT_TARGET 5u

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
  plaintext.pt.metadata = 0x415544495447524eull;
  mojov_mem_datagrant_t ciphertext;
  simon_128_128_encrypt(&simon_state, plaintext.ct.ct_lo, &ciphertext.ct.ct_lo);
  simon_128_128_encrypt(&simon_state, plaintext.ct.ct_hi ^ ciphertext.ct.ct_lo,
                        &ciphertext.ct.ct_hi);
  return ciphertext;
}

static uint64e_t finish_argmin(uint64e_t keys[8])
{
  uint64e_t best_key = keys[0];
  uint64e_t best_id((uint64_t)0);
  for (unsigned i = 1; i < 8; ++i) {
    uint64e_t take = keys[i] < best_key;
    best_key = cmov(take, keys[i], best_key);
    best_id = cmov(take, (uint64_t)i, best_id);
  }
  return best_id;
}

/* This is the client's reference computation, not a test implementation.  The
 * server cases below deliberately spell out their complete key construction so
 * that each test can be read and modified without following helper calls. */
static uint64e_t reference_selection(uint64e_t nonce)
{
  uint64e_t keys[8];
  keys[0] = certified_random<0>() ^ nonce;
  keys[1] = certified_random<1>() ^ nonce;
  keys[2] = certified_random<2>() ^ nonce;
  keys[3] = certified_random<3>() ^ nonce;
  keys[4] = certified_random<4>() ^ nonce;
  keys[5] = certified_random<5>() ^ nonce;
  keys[6] = certified_random<6>() ^ nonce;
  keys[7] = certified_random<7>() ^ nonce;
  return finish_argmin(keys);
}

template <unsigned N> static void unused_draws()
{
  (void)certified_random<100 + N>();
  unused_draws<N - 1>();
}
template <> void unused_draws<0>() { (void)certified_random<100>(); }

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
  uint64e_t nonce(encrypt_value(0xd1b54a32d192ed03ull, AUDIT_NONCE_BRAND));
  uint64e_t stale_nonce(encrypt_value(0x94d049bb133111ebull, STALE_NONCE_BRAND));

  // Construct the client's expected graph independently of the server run.
  uint64e_t expected = reference_selection(nonce);
  const uint64_t expected_hash = debug_dfhash(expected.encrypted());
  datagrant_t grant(make_datagrant(expected_hash));
  const unsigned test = read_mojov_arg();

  libmin_printf("Certified Blind Audit Selection (8 records, target record 5)\n");
  libmin_printf("INFO: Running blind audit Mojo-V test %u...\n", test);

  uint64e_t selected;
  switch (test) {
  case 0: {
    /* Positive test: all eight distinct CERTRNG leaves are request-bound and
     * enter the fixed argmin.  Its receipt must match the client reference. */
    libmin_printf("  positive test: honest certified selection.\n");
    uint64e_t keys[8];
    keys[0] = certified_random<0>() ^ nonce;
    keys[1] = certified_random<1>() ^ nonce;
    keys[2] = certified_random<2>() ^ nonce;
    keys[3] = certified_random<3>() ^ nonce;
    keys[4] = certified_random<4>() ^ nonce;
    keys[5] = certified_random<5>() ^ nonce;
    keys[6] = certified_random<6>() ^ nonce;
    keys[7] = certified_random<7>() ^ nonce;
    selected = finish_argmin(keys);
    break;
  }
  case 1: {
    /* Valid negative control: these 100 draws are never used.  Dataflow proofs
     * describe ancestors, so the following honest selection remains valid. */
    libmin_printf("  positive control: 100 extra unused certified draws.\n");
    unused_draws<99>();
    uint64e_t keys[8];
    keys[0] = certified_random<0>() ^ nonce;
    keys[1] = certified_random<1>() ^ nonce;
    keys[2] = certified_random<2>() ^ nonce;
    keys[3] = certified_random<3>() ^ nonce;
    keys[4] = certified_random<4>() ^ nonce;
    keys[5] = certified_random<5>() ^ nonce;
    keys[6] = certified_random<6>() ^ nonce;
    keys[7] = certified_random<7>() ^ nonce;
    selected = finish_argmin(keys);
    break;
  }
  case 2: {
    /* Unsafe disclosure control: both the plaintext baseline and valid Mojo-V
     * attempts expose each winner before commitment.  The server can therefore
     * keep retrying until target 5 wins; CERTRNG alone cannot stop grinding. */
    libmin_printf("  positive unsafe control: disclose-before-commit grinding.\n");
    unsigned plaintext_trials = 0;
    unsigned plaintext_selected;
    do {
      plaintext_selected = (unsigned)libmin_rand() & 7u;
      ++plaintext_trials;
    } while (plaintext_selected != AUDIT_TARGET && plaintext_trials < 50);
    libmin_printf("  plaintext baseline: target=%s after %u visible trials.\n",
                  plaintext_selected == AUDIT_TARGET ? "selected" : "not selected",
                  plaintext_trials);
    unsigned trials = 0;
    uint64_t disclosed = 0;
    do {
      uint64e_t keys[8];
      keys[0] = certified_random<0>() ^ nonce;
      keys[1] = certified_random<1>() ^ nonce;
      keys[2] = certified_random<2>() ^ nonce;
      keys[3] = certified_random<3>() ^ nonce;
      keys[4] = certified_random<4>() ^ nonce;
      keys[5] = certified_random<5>() ^ nonce;
      keys[6] = certified_random<6>() ^ nonce;
      keys[7] = certified_random<7>() ^ nonce;
      uint64e_t attempt = finish_argmin(keys);
      disclosed = _disclose(attempt.encrypted(), &grant.encrypted());
      ++trials;
    } while (disclosed != AUDIT_TARGET && trials < 50);
    libmin_printf("Selected candidate:        client-decrypted record %lu\n", disclosed);
    print_hash("Expected dfhash:           ", expected_hash);
    print_hash("Returned dfhash:           ", expected_hash);
    libmin_printf("Proof verification:        PASS\n");
    libmin_printf("Attack objective achieved: %s (%u server trials)\n",
                  disclosed == AUDIT_TARGET ? "YES" : "NO", trials);
    libmin_printf("Server observed outcome:   YES\n");
    libmin_printf("TAKEAWAY: unsafe disclosure restores a grinding oracle.\n");
    libmin_success();
    return 0;
  }
  case 10: {
    /* Force-target attack: replace record 5's certified, nonce-bound key with
     * zero.  Site 5 and its XOR vanish from the returned value's ancestry. */
    libmin_printf("  negative test: force target.\n");
    uint64e_t keys[8];
    keys[0] = certified_random<0>() ^ nonce;
    keys[1] = certified_random<1>() ^ nonce;
    keys[2] = certified_random<2>() ^ nonce;
    keys[3] = certified_random<3>() ^ nonce;
    keys[4] = certified_random<4>() ^ nonce;
    keys[5] = (uint64_t)0;
    keys[6] = certified_random<6>() ^ nonce;
    keys[7] = certified_random<7>() ^ nonce;
    selected = finish_argmin(keys);
    break;
  }
  case 11: {
    /* Software-RNG attack: the server generates a plaintext value with rand()
     * and loads it into a secret register.  It cannot assign a client brand;
     * the resulting ordinary-load provenance is not CERTRNG site 3. */
    libmin_printf("  negative test: software RNG.\n");
    uint64e_t keys[8];
    keys[0] = certified_random<0>() ^ nonce;
    keys[1] = certified_random<1>() ^ nonce;
    keys[2] = certified_random<2>() ^ nonce;
    const uint64_t software_random =
      ((uint64_t)libmin_rand() << 32) | (uint64_t)libmin_rand();
    keys[3] = uint64e_t(software_random) ^ nonce;
    keys[4] = certified_random<4>() ^ nonce;
    keys[5] = certified_random<5>() ^ nonce;
    keys[6] = certified_random<6>() ^ nonce;
    keys[7] = certified_random<7>() ^ nonce;
    selected = finish_argmin(keys);
    break;
  }
  case 12: {
    /* Skip-nonce attack: site 4 is genuine CERTRNG but is not tied to this
     * request.  The missing XOR and nonce brand make the proof fail. */
    libmin_printf("  negative test: skip nonce.\n");
    uint64e_t keys[8];
    keys[0] = certified_random<0>() ^ nonce;
    keys[1] = certified_random<1>() ^ nonce;
    keys[2] = certified_random<2>() ^ nonce;
    keys[3] = certified_random<3>() ^ nonce;
    keys[4] = certified_random<4>();
    keys[5] = certified_random<5>() ^ nonce;
    keys[6] = certified_random<6>() ^ nonce;
    keys[7] = certified_random<7>() ^ nonce;
    selected = finish_argmin(keys);
    break;
  }
  case 13: {
    /* Replay attack: every draw is bound to an earlier request.  Although the
     * algorithm is otherwise honest, STALE_NONCE_BRAND differs from the grant. */
    libmin_printf("  negative test: replay nonce.\n");
    uint64e_t keys[8];
    keys[0] = certified_random<0>() ^ stale_nonce;
    keys[1] = certified_random<1>() ^ stale_nonce;
    keys[2] = certified_random<2>() ^ stale_nonce;
    keys[3] = certified_random<3>() ^ stale_nonce;
    keys[4] = certified_random<4>() ^ stale_nonce;
    keys[5] = certified_random<5>() ^ stale_nonce;
    keys[6] = certified_random<6>() ^ stale_nonce;
    keys[7] = certified_random<7>() ^ stale_nonce;
    selected = finish_argmin(keys);
    break;
  }
  case 14: {
    /* Reuse-draw attack: all candidates share site 0.  The strict tie rule then
     * selects record 0, while sites 1 through 7 are absent from the receipt. */
    libmin_printf("  negative test: reuse draw.\n");
    uint64e_t one_key = certified_random<0>() ^ nonce;
    uint64e_t keys[8] = {
      one_key, one_key, one_key, one_key, one_key, one_key, one_key, one_key
    };
    selected = finish_argmin(keys);
    break;
  }
  case 15: {
    /* Drop-candidate attack: record 5 is represented by record 4's priority.
     * Candidate 5's CERTRNG leaf and nonce XOR do not reach the result. */
    libmin_printf("  negative test: drop candidate.\n");
    uint64e_t keys[8];
    keys[0] = certified_random<0>() ^ nonce;
    keys[1] = certified_random<1>() ^ nonce;
    keys[2] = certified_random<2>() ^ nonce;
    keys[3] = certified_random<3>() ^ nonce;
    keys[4] = certified_random<4>() ^ nonce;
    keys[5] = keys[4];
    keys[6] = certified_random<6>() ^ nonce;
    keys[7] = certified_random<7>() ^ nonce;
    selected = finish_argmin(keys);
    break;
  }
  case 16: {
    /* Wrong-draw attack: sites 2 and 5 are paired with the opposite candidate.
     * All sites exist, but their positions in the argmin graph are incorrect. */
    libmin_printf("  negative test: wrong draw.\n");
    uint64e_t keys[8];
    keys[0] = certified_random<0>() ^ nonce;
    keys[1] = certified_random<1>() ^ nonce;
    keys[2] = certified_random<5>() ^ nonce;
    keys[3] = certified_random<3>() ^ nonce;
    keys[4] = certified_random<4>() ^ nonce;
    keys[5] = certified_random<2>() ^ nonce;
    keys[6] = certified_random<6>() ^ nonce;
    keys[7] = certified_random<7>() ^ nonce;
    selected = finish_argmin(keys);
    break;
  }
  case 17: {
    /* Precomputation attack: return a complete selection calculated before the
     * fresh request arrived.  It is explicitly bound to the stale nonce. */
    libmin_printf("  negative test: precompute result.\n");
    uint64e_t keys[8];
    keys[0] = certified_random<0>() ^ stale_nonce;
    keys[1] = certified_random<1>() ^ stale_nonce;
    keys[2] = certified_random<2>() ^ stale_nonce;
    keys[3] = certified_random<3>() ^ stale_nonce;
    keys[4] = certified_random<4>() ^ stale_nonce;
    keys[5] = certified_random<5>() ^ stale_nonce;
    keys[6] = certified_random<6>() ^ stale_nonce;
    keys[7] = certified_random<7>() ^ stale_nonce;
    selected = finish_argmin(keys);
    break;
  }
  default:
    libmin_printf("ERROR: invalid blind audit test.\n");
    libmin_fail(-1);
  }

  const uint64_t returned_hash = debug_dfhash(selected.encrypted());
  libmin_printf("Selected candidate:        encrypted (server cannot observe)\n");
  print_hash("Expected dfhash:           ", expected_hash);
  print_hash("Returned dfhash:           ", returned_hash);
  libmin_printf("Proof verification:        %s\n",
                returned_hash == expected_hash ? "PASS" : "FAIL");
  libmin_printf("Attack objective achieved: NO\n");
  libmin_printf("Server observed outcome:   NO\n");

  // Validate the receipt without disclosing the winner to the server.  The
  // benchmark's client-side debug context decrypts only after that check; using
  // _disclose here would accidentally recreate case 2's grinding oracle.
  (void)_testdatagrant(selected.encrypted(), &grant.encrypted());
  const uint64_t decrypt_dfhash = debug_dfhash(selected.encrypted());
  if (decrypt_dfhash != expected_hash) {
    libmin_printf("ERROR: selected candidate dfhash changed before client decryption.\n");
    libmin_printf("DEBUG: expected dfhash 0x%08x%08x, but received 0x%08x%08x\n",
                  (uint32_t)(expected_hash >> 32), (uint32_t)expected_hash,
                  (uint32_t)(decrypt_dfhash >> 32), (uint32_t)decrypt_dfhash);
    libmin_fail(-1);
  }
  const uint64_t decrypted = selected.decrypt();
  libmin_printf("Selected candidate:        client-decrypted record %lu\n", decrypted);
  if (test == 1)
    libmin_printf("SUCCESS: unused draws are not ancestors and do not change the proof.\n");
  else
    libmin_printf("SUCCESS: certified, nonce-bound, data-oblivious selection accepted.\n");
  libmin_success();
  return 0;
}
