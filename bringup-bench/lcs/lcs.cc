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
#define N_STR_PAIRS 10

struct LcsResult {
  uint64e_t start_index;
  uint64e_t length;
};

static uint64_t u64(uint64e_t v) { return v.decrypt(); }

static stringe_t
make_encrypted_string(const char *plain, uint64_t len)
{
  stringe_t out(MAX_STR_LEN);
  for (uint64_t i = 0; i < len; ++i)
    out.push_back(uint8e_t((uint8_t)plain[i]));
  return out;
}

static uint64e_t
is_better_choice(uint64e_t cand_len, uint64e_t cand_start, uint64e_t cur_len, uint64e_t cur_start)
{
  uint64e_t longer = cand_len > cur_len;
  uint64e_t same_len = cand_len == cur_len;
  uint64e_t earlier = cand_start < cur_start;
  return longer || (same_len && earlier);
}

static LcsResult
lcs_start_and_length(const stringe_t &lhs, const stringe_t &rhs, uint64_t lhs_len, uint64_t rhs_len)
{
  uint64e_t dp[MAX_STR_LEN + 1][MAX_STR_LEN + 1];
  uint64e_t start[MAX_STR_LEN + 1][MAX_STR_LEN + 1];

  for (uint64_t i = 0; i <= lhs_len; ++i)
  {
    for (uint64_t j = 0; j <= rhs_len; ++j)
    {
      dp[i][j] = 0;
      start[i][j] = lhs_len;
    }
  }

  for (uint64_t i = 1; i <= lhs_len; ++i)
  {
    for (uint64_t j = 1; j <= rhs_len; ++j)
    {
      uint64e_t up_len = dp[i - 1][j];
      uint64e_t up_start = start[i - 1][j];
      uint64e_t left_len = dp[i][j - 1];
      uint64e_t left_start = start[i][j - 1];

      uint64e_t best_len = up_len;
      uint64e_t best_start = up_start;

      uint64e_t left_better = is_better_choice(left_len, left_start, best_len, best_start);
      best_len = cmov(left_better, left_len, best_len);
      best_start = cmov(left_better, left_start, best_start);

      uint64e_t match = lhs[i - 1] == rhs[j - 1];
      uint64e_t diag_len_prev = dp[i - 1][j - 1];
      uint64e_t diag_len = diag_len_prev + 1;
      uint64e_t diag_start = cmov(diag_len_prev == 0, uint64e_t(i - 1), start[i - 1][j - 1]);

      uint64e_t diag_better = is_better_choice(diag_len, diag_start, best_len, best_start);
      uint64e_t take_diag = match && diag_better;
      best_len = cmov(take_diag, diag_len, best_len);
      best_start = cmov(take_diag, diag_start, best_start);

      dp[i][j] = best_len;
      start[i][j] = best_start;
    }
  }

  LcsResult ret;
  ret.start_index = cmov(dp[lhs_len][rhs_len] == 0, uint64e_t(lhs_len), start[lhs_len][rhs_len]);
  ret.length = dp[lhs_len][rhs_len];
  return ret;
}

static void
lcs_string_plain(const char *lhs, const char *rhs, uint64_t lhs_len, uint64_t rhs_len, char out[MAX_STR_LEN + 1])
{
  uint16_t dp[MAX_STR_LEN + 1][MAX_STR_LEN + 1];

  for (uint64_t i = 0; i <= lhs_len; ++i)
  {
    for (uint64_t j = 0; j <= rhs_len; ++j)
      dp[i][j] = 0;
  }

  for (uint64_t i = 1; i <= lhs_len; ++i)
  {
    for (uint64_t j = 1; j <= rhs_len; ++j)
    {
      if (lhs[i - 1] == rhs[j - 1])
        dp[i][j] = dp[i - 1][j - 1] + 1;
      else
        dp[i][j] = (dp[i - 1][j] >= dp[i][j - 1]) ? dp[i - 1][j] : dp[i][j - 1];
    }
  }

  uint64_t i = lhs_len;
  uint64_t j = rhs_len;
  uint64_t pos = dp[lhs_len][rhs_len];
  out[pos] = '\0';

  while (i > 0 && j > 0)
  {
    if (lhs[i - 1] == rhs[j - 1])
    {
      out[pos - 1] = lhs[i - 1];
      pos--;
      i--;
      j--;
    }
    else if (dp[i - 1][j] >= dp[i][j - 1])
      i--;
    else
      j--;
  }
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

  static const char *lhs_plain[N_STR_PAIRS] = {
    "ABCBDAB",
    "banana",
    "XMJYAUZ",
    "stone",
    "mojov",
    "encrypt",
    "subsequenceanalysisbenchmark",
    "dataprivacypreservingsystems",
    "abcdefghijklmnoqrstuvwxyz",
    "ttttggccaattggccaattggccaa"
  };

  static const char *rhs_plain[N_STR_PAIRS] = {
    "BDCABA",
    "ananas",
    "MZJAWXU",
    "longest",
    "vmojo",
    "secret",
    "securesequenceanalyticsmodel",
    "privacyawarecomputingsystems",
    "acegikmoqsuwy",
    "ggccaattggccaatt"
  };

  static const uint64_t lhs_len[N_STR_PAIRS] = {7, 6, 7, 5, 5, 7, 28, 27, 25, 26};
  static const uint64_t rhs_len[N_STR_PAIRS] = {6, 6, 7, 7, 5, 6, 26, 26, 13, 16};
  static const uint64_t expected_start[N_STR_PAIRS] = {1, 1, 1, 2, 0, 0, 0, 4, 0, 4};
  static const uint64_t expected_len[N_STR_PAIRS] = {4, 5, 4, 3, 4, 4, 18, 19, 13, 16};
  static const char *expected_lcs[N_STR_PAIRS] = {
    "BCBA",
    "anana",
    "MJAU",
    "one",
    "mojo",
    "ecrt",
    "susequenceanalyise",
    "privacyreingsystems",
    "acegikmoqsuwy",
    "ggccaattggccaatt"
  };

  uint64_t total_len = 0;
  uint64_t total_start = 0;
  uint64_t failures = 0;

  libmin_printf("LCS: processing %d encrypted string pairs\n", N_STR_PAIRS);

  for (uint64_t i = 0; i < N_STR_PAIRS; ++i)
  {
    stringe_t lhs = make_encrypted_string(lhs_plain[i], lhs_len[i]);
    stringe_t rhs = make_encrypted_string(rhs_plain[i], rhs_len[i]);

    LcsResult res = lcs_start_and_length(lhs, rhs, lhs_len[i], rhs_len[i]);
    uint64_t start = u64(res.start_index);
    uint64_t len = u64(res.length);

    char lcs_buf[MAX_STR_LEN + 1];
    lcs_string_plain(lhs_plain[i], rhs_plain[i], lhs_len[i], rhs_len[i], lcs_buf);

    total_start += start;
    total_len += len;

    failures += (start != expected_start[i]) ? 1 : 0;
    failures += (len != expected_len[i]) ? 1 : 0;
    failures += (libmin_strcmp(lcs_buf, expected_lcs[i]) != 0) ? 1 : 0;

    libmin_printf("LCS pair[%lu]: A=\"%s\" B=\"%s\" => start=%lu len=%lu lcs=\"%s\"\n",
      (unsigned long)i, lhs_plain[i], rhs_plain[i], (unsigned long)start, (unsigned long)len, lcs_buf);
  }

  libmin_printf("LCS totals: start_sum=%lu len_sum=%lu\n", (unsigned long)total_start, (unsigned long)total_len);

  if (failures != 0)
  {
    libmin_printf("LCS FAIL: %lu mismatches\n", (unsigned long)failures);
    return -1;
  }

  libmin_printf("LCS PASS: all expected start/len/string checks passed\n");

  libmin_success();
  return 0;
}
