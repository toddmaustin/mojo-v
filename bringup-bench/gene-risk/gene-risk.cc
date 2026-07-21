#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-proofcarrying.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_proofcarrying_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_proofcarrying_fp64_t
#include "mojov-exo.h"

#define GENE_MARKERS 8u
#define MARKER_BRAND_BASE 0x47454e454d4b5200ull
#define SCORE_BRAND_BASE 0x47454e4553435200ull
#define BUCKET_BRAND_BASE 0x47454e4542554300ull

static uint128_t simon_key = SIMON128_KEY;
static simon_state_t simon_state;

static mojov_mem_proofcarrying_u64_t
encrypt_u64(uint64_t value, uint64_t brand)
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
    libmin_printf("ERROR: proof-carrying debug decrypt failed for gene-risk value.\n");
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
  ptval.pt.metadata = 0x47454e455249534bull;
  mojov_mem_datagrant_t ctval;

  simon_128_128_encrypt(&simon_state, ptval.ct.ct_lo, &ctval.ct.ct_lo);
  simon_128_128_encrypt(&simon_state, (ptval.ct.ct_hi ^ ctval.ct.ct_lo), &ctval.ct.ct_hi);

  return ctval;
}

static void
compute_encrypted_gene_risk(const mojov_mem_proofcarrying_u64_t encrypted_markers[GENE_MARKERS],
                            uint64e_t *score_out,
                            uint64e_t *bucket_out)
{
  // Each marker is a toy SNP dosage encoded as 0, 1, or 2 copies of a risk
  // allele. In a real genomic workload these marker values would be read from
  // an encrypted genotype dataset; here they arrive already sealed in
  // proof-carrying Mojo-V ciphertexts so the kernel can use them but cannot
  // safely disclose them.
  //
  // The weights model a simplified polygenic risk score (PRS): each SNP
  // contributes marker[i] * weights[i] to the final score. These are small
  // integer weights rather than clinically meaningful coefficients so the
  // benchmark stays deterministic, compact, and runnable without FP support.
  static const uint64_t weights[GENE_MARKERS] = { 9u, 12u, 7u, 15u, 4u, 11u, 6u, 10u };

  // Accumulate the PRS entirely in encrypted/proof-carrying form. The loop is
  // the privacy-preserving "kernel" of the benchmark: it touches every raw SNP
  // marker and performs useful weighted arithmetic, but the only value that is
  // intended to leave the encrypted domain is the final aggregate score (and,
  // optionally, the risk bucket derived from it).
  uint64e_t score(encrypt_u64(0u, SCORE_BRAND_BASE));
  for (unsigned i = 0; i < GENE_MARKERS; i++)
  {
    uint64e_t marker(encrypted_markers[i]);
    uint64e_t weighted = marker * weights[i];
    score = score + weighted;
  }

  // Convert the encrypted numeric score into an encrypted categorical result:
  //   0 = low, 1 = medium, 2 = high.
  //
  // The comparisons produce encrypted predicates, and cmov() uses those
  // predicates to select the encrypted category without disclosing which branch
  // was taken. This demonstrates disclosure minimization: a data grant can
  // authorize the clinical category without authorizing either the raw genome
  // markers or the intermediate threshold predicates.
  uint64e_t low(encrypt_u64(0u, BUCKET_BRAND_BASE));
  uint64e_t medium(encrypt_u64(1u, BUCKET_BRAND_BASE + 1u));
  uint64e_t high(encrypt_u64(2u, BUCKET_BRAND_BASE + 2u));
  uint64e_t at_least_medium = score > 99u;
  uint64e_t at_least_high = score > 159u;
  uint64e_t bucket = cmov(at_least_medium, medium, low);
  bucket = cmov(at_least_high, high, bucket);

  *score_out = score;
  *bucket_out = bucket;
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
  // Toy patient genotype: eight encrypted SNP marker dosages. The plaintext
  // literals only exist at setup time to create deterministic benchmark input;
  // after encryption, all risk computation uses mojov-exo encrypted integer
  // values. The positive test must never disclose any element of this array,
  // and the negative tests prove that a grant for the derived score does not
  // authorize disclosure of a raw marker.
  static const uint64_t raw_markers[GENE_MARKERS] = { 2u, 1u, 0u, 2u, 1u, 2u, 0u, 1u };

  if (mojov_configure_kmsm_from_dc_proof() != 0)
    return -1;
  if (mojov_enable_and_verify() != 0)
    return -1;
  if (!simon_128_128_keyexpand(&simon_state, simon_key, 68))
    return -1;
  if (debug_context(SIMON128_KEY, CONTRACT_SIG) != 0)
    return -1;

  libmin_srand(42);

  mojov_mem_proofcarrying_u64_t encrypted_markers[GENE_MARKERS];
  for (unsigned i = 0; i < GENE_MARKERS; i++)
    encrypted_markers[i] = encrypt_u64(raw_markers[i], MARKER_BRAND_BASE + i);

  // First pass: run the kernel only to compute the proof-carrying hashes that
  // the data owner will authorize. This mirrors the private-auction benchmark:
  // data grants are tied to the exact computation outputs, not to variable
  // names, plaintext values, or broad application authority.
  uint64e_t expected_score;
  uint64e_t expected_bucket;
  compute_encrypted_gene_risk(encrypted_markers, &expected_score, &expected_bucket);

  datagrant_t score_grant(make_datagrant(debug_dfhash_u64(expected_score.encrypted())));
  datagrant_t bucket_grant(make_datagrant(debug_dfhash_u64(expected_bucket.encrypted())));

  // Second pass: recompute the same encrypted kernel outputs and compare their
  // proof-carrying hashes with the grants. Only values whose hashes match their
  // grants are eligible for _disclose(); changing a marker, deriving score + 1,
  // or trying to reveal an intermediate predicate changes the hash and traps.
  uint64e_t score;
  uint64e_t bucket;
  compute_encrypted_gene_risk(encrypted_markers, &score, &bucket);

  const datagrant_t::plaintext_type score_grant_plaintext = score_grant.decrypt();
  const datagrant_t::plaintext_type bucket_grant_plaintext = bucket_grant.decrypt();
  check_dfhash_integrity("risk_score", score_grant_plaintext.dfhash, debug_dfhash_u64(score.encrypted()));
  check_dfhash_integrity("risk_bucket", bucket_grant_plaintext.dfhash, debug_dfhash_u64(bucket.encrypted()));

  const uint16_t mojov_arg = read_mojov_arg();

  libmin_printf("Gene-risk safe disclosure demo\n");
  libmin_printf("  encrypted SNP markers: %u\n", GENE_MARKERS);
  libmin_printf("  debug: risk score grant valid: %s\n", score_grant.is_valid() ? "yes" : "no");
  libmin_printf("  debug: risk bucket grant valid: %s\n", bucket_grant.is_valid() ? "yes" : "no");
  libmin_printf("INFO: Running gene-risk Mojo-V test %u...\n", (uint32_t)mojov_arg);

  switch (mojov_arg)
  {
  case 0:
  {
    (void)_testdatagrant(score.encrypted(), &score_grant.encrypted());
    (void)_testdatagrant(bucket.encrypted(), &bucket_grant.encrypted());

    const uint64_t disclosed_score = _disclose(score.encrypted(), &score_grant.encrypted());
    const uint64_t disclosed_bucket = _disclose(bucket.encrypted(), &bucket_grant.encrypted());

    libmin_printf("  disclosed polygenic risk score: %lu\n", disclosed_score);
    libmin_printf("  disclosed risk bucket: %lu (0=low, 1=medium, 2=high)\n", disclosed_bucket);
    libmin_printf("SUCCESS: disclosed risk_score (96) and risk bucket (low).\n");
    libmin_printf("SUCCESS: raw genetic markers remain encrypted; only granted clinical result was disclosed.\n");

    if (disclosed_score != 96u || disclosed_bucket != 0u)
    {
      libmin_printf("ERROR: gene-risk disclosed the wrong risk result.\n");
      libmin_fail(-1);
    }
    break;
  }

  case 1:
    libmin_printf("  negative test: disclose risk_bucket with risk_score_grant.\n");
    (void)_testdatagrant(score.encrypted(), &score_grant.encrypted());
    (void)_disclose(bucket.encrypted(), &score_grant.encrypted());
    negative_test_failed("risk bucket mismatched datagrant test");
    break;

  case 2:
    libmin_printf("  negative test: disclose encrypted_markers[0] with risk_score_grant.\n");
    (void)_testdatagrant(score.encrypted(), &score_grant.encrypted());
    (void)_disclose(encrypted_markers[0], &score_grant.encrypted());
    negative_test_failed("raw marker mismatched datagrant test");
    break;

  case 3:
    libmin_printf("  negative test: disclose risk_score with risk_bucket_grant.\n");
    (void)_testdatagrant(bucket.encrypted(), &bucket_grant.encrypted());
    (void)_disclose(score.encrypted(), &bucket_grant.encrypted());
    negative_test_failed("risk score mismatched datagrant test");
    break;

  case 4:
  {
    libmin_printf("  negative test: disclose risk_score with bogus datagrant ciphertext.\n");
    mojov_mem_datagrant_t *bogus_grant = (mojov_mem_datagrant_t *)&encrypted_markers[1];
    (void)_testdatagrant(score.encrypted(), bogus_grant);
    (void)_disclose(score.encrypted(), bogus_grant);
    negative_test_failed("bogus datagrant ciphertext test");
    break;
  }

  case 5:
  {
    libmin_printf("  negative test: disclose risk_score with tampered datagrant dfhash.\n");
    datagrant_t tampered_grant(make_datagrant(score_grant_plaintext.dfhash ^ 1u));
    (void)_testdatagrant(score.encrypted(), &tampered_grant.encrypted());
    (void)_disclose(score.encrypted(), &tampered_grant.encrypted());
    negative_test_failed("tampered datagrant dfhash test");
    break;
  }

  case 6:
  {
    libmin_printf("  negative test: disclose modified genome score with stale risk_score_grant.\n");
    mojov_mem_proofcarrying_u64_t modified_markers[GENE_MARKERS];
    for (unsigned i = 0; i < GENE_MARKERS; i++)
      modified_markers[i] = encrypted_markers[i];
    modified_markers[7] = encrypt_u64(2u, MARKER_BRAND_BASE + 107u);

    uint64e_t modified_score;
    uint64e_t modified_bucket;
    compute_encrypted_gene_risk(modified_markers, &modified_score, &modified_bucket);

    (void)_testdatagrant(score.encrypted(), &score_grant.encrypted());
    (void)_disclose(modified_score.encrypted(), &score_grant.encrypted());
    negative_test_failed("stale risk score datagrant test");
    break;
  }

  case 7:
  {
    libmin_printf("  negative test: disclose risk_score plus encrypted_one with risk_score_grant.\n");
    uint64e_t encrypted_one(encrypt_u64(1u, SCORE_BRAND_BASE + 0x100u));
    uint64e_t derived_score = score + encrypted_one;
    (void)_testdatagrant(score.encrypted(), &score_grant.encrypted());
    (void)_disclose(derived_score.encrypted(), &score_grant.encrypted());
    negative_test_failed("encrypted-one derived risk score datagrant test");
    break;
  }

  case 8:
  {
    libmin_printf("  negative test: disclose risk_score plus immediate one with risk_score_grant.\n");
    uint64e_t incremented_score = score + 1u;
    (void)_testdatagrant(score.encrypted(), &score_grant.encrypted());
    (void)_disclose(incremented_score.encrypted(), &score_grant.encrypted());
    negative_test_failed("immediate-one derived risk score datagrant test");
    break;
  }

  case 9:
  {
    libmin_printf("  negative test: disclose intermediate high-risk predicate with risk_score_grant.\n");
    uint64e_t high_risk_predicate = score > 159u;
    (void)_testdatagrant(score.encrypted(), &score_grant.encrypted());
    (void)_disclose(high_risk_predicate.encrypted(), &score_grant.encrypted());
    negative_test_failed("intermediate predicate datagrant test");
    break;
  }

  default:
    libmin_printf("ERROR: invalid gene-risk test (%u).\n", (uint32_t)mojov_arg);
    libmin_fail(-1);
  }

  libmin_success();
  return 0;
}
