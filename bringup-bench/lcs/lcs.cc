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

struct MatchResult {
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

static MatchResult
lcseq_start_and_length(const stringe_t &lhs, const stringe_t &rhs, uint64_t lhs_len, uint64_t rhs_len)
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

  MatchResult ret;
  ret.start_index = cmov(dp[lhs_len][rhs_len] == 0, uint64e_t(lhs_len), start[lhs_len][rhs_len]);
  ret.length = dp[lhs_len][rhs_len];
  return ret;
}

static MatchResult
lcsub_start_and_length(const stringe_t &lhs, const stringe_t &rhs, uint64_t lhs_len, uint64_t rhs_len)
{
  uint64e_t dp[MAX_STR_LEN + 1][MAX_STR_LEN + 1];
  uint64e_t best_len = 0;
  uint64e_t best_start = lhs_len;

  for (uint64_t i = 0; i <= lhs_len; ++i)
  {
    for (uint64_t j = 0; j <= rhs_len; ++j)
      dp[i][j] = 0;
  }

  for (uint64_t i = 1; i <= lhs_len; ++i)
  {
    for (uint64_t j = 1; j <= rhs_len; ++j)
    {
      uint64e_t match = lhs[i - 1] == rhs[j - 1];
      uint64e_t cand_len = dp[i - 1][j - 1] + 1;
      uint64e_t run_len = cmov(match, cand_len, uint64e_t(0));
      dp[i][j] = run_len;

      uint64e_t cand_start = uint64e_t(i) - run_len;
      uint64e_t better = is_better_choice(run_len, cand_start, best_len, best_start);
      uint64e_t take = (run_len > 0) && better;
      best_len = cmov(take, run_len, best_len);
      best_start = cmov(take, cand_start, best_start);
    }
  }

  MatchResult ret;
  ret.start_index = cmov(best_len == 0, uint64e_t(lhs_len), best_start);
  ret.length = best_len;
  return ret;
}

static void
lcseq_string_plain(const char *lhs, const char *rhs, uint64_t lhs_len, uint64_t rhs_len, char out[MAX_STR_LEN + 1])
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

static void
slice_plain(const char *src, uint64_t start, uint64_t len, char out[MAX_STR_LEN + 1])
{
  for (uint64_t i = 0; i < len; ++i)
    out[i] = src[start + i];
  out[len] = '\0';
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

  static const uint64_t expected_seq_start[N_STR_PAIRS] = {1, 1, 1, 2, 0, 0, 0, 4, 0, 4};
  static const uint64_t expected_seq_len[N_STR_PAIRS] = {4, 5, 4, 3, 4, 4, 18, 19, 13, 16};
  static const char *expected_seq[N_STR_PAIRS] = {
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

  static const uint64_t expected_sub_start[N_STR_PAIRS] = {0, 1, 0, 0, 0, 2, 3, 18, 14, 4};
  static const uint64_t expected_sub_len[N_STR_PAIRS] = {2, 5, 1, 2, 4, 2, 13, 10, 2, 16};
  static const char *expected_sub[N_STR_PAIRS] = {
    "AB",
    "anana",
    "X",
    "st",
    "mojo",
    "cr",
    "sequenceanaly",
    "ingsystems",
    "oq",
    "ggccaattggccaatt"
  };

  uint64_t total_seq_len = 0;
  uint64_t total_seq_start = 0;
  uint64_t total_sub_len = 0;
  uint64_t total_sub_start = 0;
  uint64_t failures = 0;

  libmin_printf("LCS: processing %d encrypted string pairs\n", N_STR_PAIRS);

  for (uint64_t i = 0; i < N_STR_PAIRS; ++i)
  {
    uint64_t lhs_len = libmin_strlen(lhs_plain[i]);
    uint64_t rhs_len = libmin_strlen(rhs_plain[i]);

    if (lhs_len > MAX_STR_LEN || rhs_len > MAX_STR_LEN)
    {
      libmin_printf("LCS FAIL: pair[%lu] input exceeds MAX_STR_LEN=%d (lhs_len=%lu rhs_len=%lu)\n",
        (unsigned long)i, MAX_STR_LEN, (unsigned long)lhs_len, (unsigned long)rhs_len);
      return -1;
    }

    stringe_t lhs = make_encrypted_string(lhs_plain[i], lhs_len);
    stringe_t rhs = make_encrypted_string(rhs_plain[i], rhs_len);

    MatchResult lcseq = lcseq_start_and_length(lhs, rhs, lhs_len, rhs_len);
    MatchResult lcsub = lcsub_start_and_length(lhs, rhs, lhs_len, rhs_len);

    uint64_t seq_start = u64(lcseq.start_index);
    uint64_t seq_len = u64(lcseq.length);
    uint64_t sub_start = u64(lcsub.start_index);
    uint64_t sub_len = u64(lcsub.length);

    char lcseq_buf[MAX_STR_LEN + 1];
    char lcsub_buf[MAX_STR_LEN + 1];
    lcseq_string_plain(lhs_plain[i], rhs_plain[i], lhs_len, rhs_len, lcseq_buf);
    slice_plain(lhs_plain[i], sub_start, sub_len, lcsub_buf);

    total_seq_start += seq_start;
    total_seq_len += seq_len;
    total_sub_start += sub_start;
    total_sub_len += sub_len;

    uint64_t pair_failures = 0;
    uint64_t seq_start_mismatch = (seq_start != expected_seq_start[i]) ? 1 : 0;
    uint64_t seq_len_mismatch = (seq_len != expected_seq_len[i]) ? 1 : 0;
    uint64_t seq_str_mismatch = (libmin_strcmp(lcseq_buf, expected_seq[i]) != 0) ? 1 : 0;
    uint64_t sub_start_mismatch = (sub_start != expected_sub_start[i]) ? 1 : 0;
    uint64_t sub_len_mismatch = (sub_len != expected_sub_len[i]) ? 1 : 0;
    uint64_t sub_str_mismatch = (libmin_strcmp(lcsub_buf, expected_sub[i]) != 0) ? 1 : 0;

    pair_failures += seq_start_mismatch;
    pair_failures += seq_len_mismatch;
    pair_failures += seq_str_mismatch;
    pair_failures += sub_start_mismatch;
    pair_failures += sub_len_mismatch;
    pair_failures += sub_str_mismatch;
    failures += pair_failures;

    libmin_printf("LCS pair[%lu]: A=\"%s\" B=\"%s\" => LCseq(start=%lu,len=%lu,LCseq=\"%s\") LCsub(start=%lu,len=%lu,LCsub=\"%s\")\n",
      (unsigned long)i, lhs_plain[i], rhs_plain[i],
      (unsigned long)seq_start, (unsigned long)seq_len, lcseq_buf,
      (unsigned long)sub_start, (unsigned long)sub_len, lcsub_buf);

    if (pair_failures != 0)
    {
      libmin_printf("  FAIL pair[%lu]: expected LCseq(start=%lu,len=%lu,LCseq=\"%s\") LCsub(start=%lu,len=%lu,LCsub=\"%s\")\n",
        (unsigned long)i,
        (unsigned long)expected_seq_start[i], (unsigned long)expected_seq_len[i], expected_seq[i],
        (unsigned long)expected_sub_start[i], (unsigned long)expected_sub_len[i], expected_sub[i]);
      if (seq_start_mismatch)
        libmin_printf("    mismatch: LCseq start expected=%lu actual=%lu\n",
          (unsigned long)expected_seq_start[i], (unsigned long)seq_start);
      if (seq_len_mismatch)
        libmin_printf("    mismatch: LCseq len expected=%lu actual=%lu\n",
          (unsigned long)expected_seq_len[i], (unsigned long)seq_len);
      if (seq_str_mismatch)
        libmin_printf("    mismatch: LCseq str expected=\"%s\" actual=\"%s\"\n",
          expected_seq[i], lcseq_buf);
      if (sub_start_mismatch)
        libmin_printf("    mismatch: LCsub start expected=%lu actual=%lu\n",
          (unsigned long)expected_sub_start[i], (unsigned long)sub_start);
      if (sub_len_mismatch)
        libmin_printf("    mismatch: LCsub len expected=%lu actual=%lu\n",
          (unsigned long)expected_sub_len[i], (unsigned long)sub_len);
      if (sub_str_mismatch)
        libmin_printf("    mismatch: LCsub str expected=\"%s\" actual=\"%s\"\n",
          expected_sub[i], lcsub_buf);
    }
  }

  libmin_printf("LCS totals: LCseq_start_sum=%lu LCseq_len_sum=%lu LCsub_start_sum=%lu LCsub_len_sum=%lu\n",
    (unsigned long)total_seq_start, (unsigned long)total_seq_len,
    (unsigned long)total_sub_start, (unsigned long)total_sub_len);

  if (failures != 0)
  {
    libmin_printf("LCS FAIL: %lu mismatches\n", (unsigned long)failures);
    return -1;
  }

  libmin_printf("LCS PASS: all expected LCseq/LCsub checks passed\n");

  libmin_success();
  return 0;
}
