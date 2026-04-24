#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"
#include "mojov-string.h"

using namespace exo;

#define MAX_STR_LEN 24
#define N_TESTS 12

static stringe_t
make_encrypted_string(const char *plain, uint64_t len)
{
  stringe_t out(MAX_STR_LEN);
  for (uint64_t i = 0; i < len; ++i)
    out.push_back(uint8e_t((uint8_t)plain[i]));
  return out;
}

static uint64e_t
is_scramble_encrypted(const stringe_t &lhs, const stringe_t &rhs, uint64_t n)
{
  if (n == 0)
    return uint64e_t(1);

  uint64e_t dp[MAX_STR_LEN + 1][MAX_STR_LEN][MAX_STR_LEN];

  for (uint64_t len = 0; len <= n; ++len)
  {
    for (uint64_t i = 0; i < n; ++i)
    {
      for (uint64_t j = 0; j < n; ++j)
        dp[len][i][j] = uint64e_t(0);
    }
  }

  for (uint64_t i = 0; i < n; ++i)
  {
    for (uint64_t j = 0; j < n; ++j)
      dp[1][i][j] = lhs[i] == rhs[j];
  }

  for (uint64_t len = 2; len <= n; ++len)
  {
    for (uint64_t i = 0; i + len <= n; ++i)
    {
      for (uint64_t j = 0; j + len <= n; ++j)
      {
        uint64e_t best = 0;

        for (uint64_t split = 1; split < len; ++split)
        {
          uint64e_t no_swap = dp[split][i][j] && dp[len - split][i + split][j + split];
          uint64e_t swap = dp[split][i][j + len - split] && dp[len - split][i + split][j];
          best = best || no_swap || swap;
        }

        dp[len][i][j] = best;
      }
    }
  }

  return dp[n][0][0];
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
    "great",
    "abcde",
    "a",
    "abcdefghijkl",
    "abab",
    "coder",
    "algorithm",
    "xy",
    "aaab",
    "night",
    "aabbcc",
    "random"
  };

  static const char *rhs_plain[N_TESTS] = {
    "rgeat",
    "caebd",
    "a",
    "efghijklabcd",
    "baba",
    "ocder",
    "logarithm",
    "yx",
    "abaa",
    "thing",
    "abcabc",
    "dorman"
  };

  static const uint64_t expected[N_TESTS] = {1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0};

  uint64e_t result_enc[N_TESTS];
  uint64e_t true_total = 0;
  uint64e_t mismatch_total = 0;

  for (uint64_t i = 0; i < N_TESTS; ++i)
  {
    uint64_t lhs_len = libmin_strlen(lhs_plain[i]);
    uint64_t rhs_len = libmin_strlen(rhs_plain[i]);

    if (lhs_len > MAX_STR_LEN || rhs_len > MAX_STR_LEN || lhs_len != rhs_len)
    {
      libmin_printf("Scrambled-Compare FAIL: invalid input pair[%lu]\n", (unsigned long)i);
      return -1;
    }

    stringe_t lhs = make_encrypted_string(lhs_plain[i], lhs_len);
    stringe_t rhs = make_encrypted_string(rhs_plain[i], rhs_len);

    uint64e_t actual = is_scramble_encrypted(lhs, rhs, lhs_len);
    result_enc[i] = actual;

    uint64e_t actual_bit = cmov(actual != uint64e_t(0), uint64e_t(1), uint64e_t(0));
    uint64e_t expected_bit = uint64e_t(expected[i]);
    true_total += actual_bit;
    mismatch_total += cmov(actual_bit != expected_bit, uint64e_t(1), uint64e_t(0));
  }

  libmin_printf("Scrambled-Compare: %d encrypted string pairs\n", N_TESTS);

  for (uint64_t i = 0; i < N_TESTS; ++i)
  {
    uint64_t actual = result_enc[i].decrypt();
    libmin_printf("scramble[%lu]: A=\"%s\" B=\"%s\" => is_scramble=%lu\n",
      (unsigned long)i, lhs_plain[i], rhs_plain[i], (unsigned long)actual);
  }

  uint64_t total_true = true_total.decrypt();
  uint64_t mismatches = mismatch_total.decrypt();

  libmin_printf("Scrambled-Compare totals: true_count=%lu mismatches=%lu\n",
    (unsigned long)total_true, (unsigned long)mismatches);

  if (mismatches != 0)
  {
    libmin_printf("Scrambled-Compare FAIL: expected signatures did not match\n");
    return -1;
  }

  libmin_printf("Scrambled-Compare PASS: encrypted dynamic-programming scramble checks match expected outcomes\n");

  libmin_success();
  return 0;
}
