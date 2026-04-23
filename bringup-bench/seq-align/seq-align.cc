#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"
#include "mojov-string.h"

using namespace exo;

#define MAX_STR_LEN 48
#define N_TESTS 16

static constexpr int64_t MATCH_SCORE = 2;
static constexpr int64_t MISMATCH_SCORE = -1;
static constexpr int64_t GAP_SCORE = -2;

static int64_t i64(int64e_t v) { return v.decrypt(); }

static stringe_t
make_encrypted_string(const char *plain, uint64_t len)
{
  stringe_t out(MAX_STR_LEN);
  for (uint64_t i = 0; i < len; ++i)
    out.push_back(uint8e_t((uint8_t)plain[i]));
  return out;
}

static int64e_t
max3(int64e_t a, int64e_t b, int64e_t c)
{
  int64e_t ab = cmov(a >= b, a, b);
  return cmov(ab >= c, ab, c);
}

static int64e_t
needleman_wunsch_score_encrypted(const stringe_t &lhs, const stringe_t &rhs, uint64_t lhs_len, uint64_t rhs_len)
{
  int64e_t dp[MAX_STR_LEN + 1][MAX_STR_LEN + 1];

  dp[0][0] = int64e_t(0);
  for (uint64_t i = 1; i <= lhs_len; ++i)
    dp[i][0] = dp[i - 1][0] + int64e_t(GAP_SCORE);
  for (uint64_t j = 1; j <= rhs_len; ++j)
    dp[0][j] = dp[0][j - 1] + int64e_t(GAP_SCORE);

  for (uint64_t i = 1; i <= lhs_len; ++i)
  {
    for (uint64_t j = 1; j <= rhs_len; ++j)
    {
      uint64e_t is_match = lhs[i - 1] == rhs[j - 1];
      int64e_t subst = cmov(is_match, int64e_t(MATCH_SCORE), int64e_t(MISMATCH_SCORE));
      int64e_t diag = dp[i - 1][j - 1] + subst;
      int64e_t up = dp[i - 1][j] + int64e_t(GAP_SCORE);
      int64e_t left = dp[i][j - 1] + int64e_t(GAP_SCORE);
      dp[i][j] = max3(diag, up, left);
    }
  }

  return dp[lhs_len][rhs_len];
}

static int64_t
needleman_wunsch_score_plain(const char *lhs, const char *rhs, uint64_t lhs_len, uint64_t rhs_len)
{
  int64_t dp[MAX_STR_LEN + 1][MAX_STR_LEN + 1];

  dp[0][0] = 0;
  for (uint64_t i = 1; i <= lhs_len; ++i)
    dp[i][0] = dp[i - 1][0] + GAP_SCORE;
  for (uint64_t j = 1; j <= rhs_len; ++j)
    dp[0][j] = dp[0][j - 1] + GAP_SCORE;

  for (uint64_t i = 1; i <= lhs_len; ++i)
  {
    for (uint64_t j = 1; j <= rhs_len; ++j)
    {
      int64_t subst = (lhs[i - 1] == rhs[j - 1]) ? MATCH_SCORE : MISMATCH_SCORE;
      int64_t diag = dp[i - 1][j - 1] + subst;
      int64_t up = dp[i - 1][j] + GAP_SCORE;
      int64_t left = dp[i][j - 1] + GAP_SCORE;

      int64_t best = diag;
      if (up > best)
        best = up;
      if (left > best)
        best = left;
      dp[i][j] = best;
    }
  }

  return dp[lhs_len][rhs_len];
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

  static const char *lhs_plain[N_TESTS] = {
    "GATTACA",
    "ACTG",
    "AAAA",
    "ACGTACGT",
    "TTAACCGG",
    "SUNDAY",
    "ALIGNMENT",
    "ENCRYPTEDSTRING",
    "HELLOWORLD",
    "BIOINFORMATICS",
    "A",
    "",
    "TTTTTTTT",
    "MATRIXCOMPUTE",
    "NEEDLEMANWUNSCH",
    "CAGTCAGTGGAC"
  };

  static const char *rhs_plain[N_TESTS] = {
    "GCATGCU",
    "ACGT",
    "TTTT",
    "ACGACG",
    "TTACGG",
    "SATURDAY",
    "SLIME",
    "ENCRYPTIONRING",
    "YELLOWBIRD",
    "INFORMATION",
    "A",
    "MOJOV",
    "",
    "MATRICESCOMPUTED",
    "NEEDLEWUNSCH",
    "CAGTAGTAGGAC"
  };

  uint64_t failures = 0;
  int64_t score_sum = 0;

  libmin_printf("Seq-Align (Needleman-Wunsch): %d encrypted string pairs (match=%ld mismatch=%ld gap=%ld)\n",
    N_TESTS, (long)MATCH_SCORE, (long)MISMATCH_SCORE, (long)GAP_SCORE);

  for (uint64_t i = 0; i < N_TESTS; ++i)
  {
    uint64_t lhs_len = libmin_strlen(lhs_plain[i]);
    uint64_t rhs_len = libmin_strlen(rhs_plain[i]);

    if (lhs_len > MAX_STR_LEN || rhs_len > MAX_STR_LEN)
    {
      libmin_printf("NW FAIL: pair[%lu] input exceeds MAX_STR_LEN=%d (lhs_len=%lu rhs_len=%lu)\n",
        (unsigned long)i, MAX_STR_LEN, (unsigned long)lhs_len, (unsigned long)rhs_len);
      return -1;
    }

    stringe_t lhs = make_encrypted_string(lhs_plain[i], lhs_len);
    stringe_t rhs = make_encrypted_string(rhs_plain[i], rhs_len);

    int64_t enc_score = i64(needleman_wunsch_score_encrypted(lhs, rhs, lhs_len, rhs_len));
    int64_t ref_score = needleman_wunsch_score_plain(lhs_plain[i], rhs_plain[i], lhs_len, rhs_len);

    score_sum += enc_score;

    libmin_printf("NW pair[%lu]: A=\"%s\" B=\"%s\" => score=%ld\n",
      (unsigned long)i, lhs_plain[i], rhs_plain[i], (long)enc_score);

    if (enc_score != ref_score)
    {
      failures++;
      libmin_printf("  FAIL pair[%lu]: expected=%ld actual=%ld\n",
        (unsigned long)i, (long)ref_score, (long)enc_score);
    }
  }

  libmin_printf("NW totals: score_sum=%ld\n", (long)score_sum);

  if (failures != 0)
  {
    libmin_printf("NW FAIL: %lu score mismatches\n", (unsigned long)failures);
    return -1;
  }

  libmin_printf("NW PASS: encrypted scores match plaintext reference across all test inputs\n");

  libmin_success();
  return 0;
}
