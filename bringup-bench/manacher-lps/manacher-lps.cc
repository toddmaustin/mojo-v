#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"
#include "mojov-string.h"

using namespace exo;

#define MAX_STR_LEN 64
#define N_STRINGS 10

struct PalResult {
  uint64e_t start_index;
  uint64e_t length;
};

static stringe_t
make_encrypted_string(const char *plain, uint64_t len)
{
  stringe_t out(MAX_STR_LEN);
  for (uint64_t i = 0; i < len; ++i)
    out.push_back(uint8e_t((uint8_t)plain[i]));
  return out;
}


static PalResult
longest_palindrome_manacher_encrypted(const stringe_t &s, uint64_t n)
{
  uint64e_t best_len = 0;
  uint64e_t best_start = n;

  for (uint64_t start = 0; start < n; ++start)
  {
    for (uint64_t len = 1; len <= (n - start); ++len)
    {
      uint64e_t is_pal = 1;
      const uint64_t half = len >> 1;

      for (uint64_t k = 0; k < MAX_STR_LEN; ++k)
      {
        if (k < half)
        {
          const uint64_t left_idx = start + k;
          const uint64_t right_idx = start + len - 1 - k;
          const uint64e_t eq = s[left_idx] == s[right_idx];
          is_pal = cmov(!eq, uint64e_t(0), is_pal);
        }
      }

      const uint64e_t cand_len = uint64e_t(len);
      const uint64e_t cand_start = uint64e_t(start);
      const uint64e_t longer = cand_len > best_len;
      const uint64e_t tie_break = (cand_len == best_len) && (cand_start < best_start);
      const uint64e_t better = is_pal && (longer || tie_break);

      best_len = cmov(better, cand_len, best_len);
      best_start = cmov(better, cand_start, best_start);
    }
  }

  PalResult ret;
  ret.start_index = cmov(best_len == 0, uint64e_t(n), best_start);
  ret.length = best_len;
  return ret;
}

static void
plain_longest_palindrome(const char *s, uint64_t n, uint64_t *out_start, uint64_t *out_len)
{
  uint64_t best_start = n;
  uint64_t best_len = 0;

  for (uint64_t c = 0; c < n; ++c)
  {
    uint64_t l = c;
    uint64_t r = c;
    while (r < n && s[l] == s[r])
    {
      uint64_t len = r - l + 1;
      if (len > best_len || (len == best_len && l < best_start))
      {
        best_len = len;
        best_start = l;
      }

      if (l == 0 || r + 1 >= n)
        break;
      --l;
      ++r;
    }

    l = c;
    r = c + 1;
    while (r < n && s[l] == s[r])
    {
      uint64_t len = r - l + 1;
      if (len > best_len || (len == best_len && l < best_start))
      {
        best_len = len;
        best_start = l;
      }

      if (l == 0 || r + 1 >= n)
        break;
      --l;
      ++r;
    }
  }

  *out_start = (best_len == 0) ? n : best_start;
  *out_len = best_len;
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

  static const char *plain_inputs[N_STRINGS] = {
    "bananas",
    "forgeeksskeegfor",
    "abacdfgdcaba",
    "abbaabba",
    "mojovracecarbenchmark",
    "abcdef",
    "aaaaabaaaa",
    "neveroddoreven",
    "xyzyx123321",
    "tacocatlevelstats"
  };

  uint64_t failures = 0;
  uint64_t got_starts[N_STRINGS];
  uint64_t got_lens[N_STRINGS];

  libmin_printf("manacher-lps: processing %d encrypted strings\n", N_STRINGS);

  for (uint64_t i = 0; i < N_STRINGS; ++i)
  {
    uint64_t n = libmin_strlen(plain_inputs[i]);
    if (n > MAX_STR_LEN)
      return -1;

    stringe_t secret_s = make_encrypted_string(plain_inputs[i], n);
    PalResult secret_result = longest_palindrome_manacher_encrypted(secret_s, n);

    uint64_t got_start = secret_result.start_index.decrypt();
    uint64_t got_len = secret_result.length.decrypt();

    uint64_t exp_start = 0;
    uint64_t exp_len = 0;
    plain_longest_palindrome(plain_inputs[i], n, &exp_start, &exp_len);

    libmin_printf("manacher-lps[%2lu]: start=%2lu len=%2lu   ", i, got_start, got_len);

    got_starts[i] = got_start;
    got_lens[i] = got_len;

    if (got_start != exp_start || got_len != exp_len)
    {
      failures++;
      libmin_printf("\nERROR: mismatch i=%2lu expected=(%2lu,%2lu) got=(%lu,%lu)\n",
        i, exp_start, exp_len, got_start, got_len);
    }
    else
    {
      char palindrome[MAX_STR_LEN + 1];
      uint64_t pal_len = got_lens[i];
      uint64_t pal_start = got_starts[i];

      for (uint64_t j = 0; j < pal_len; ++j)
        palindrome[j] = plain_inputs[i][pal_start + j];
      palindrome[pal_len] = '\0';

      libmin_printf("  string[%2lu]=\"%s\" longest=\"%s\"\n", i, plain_inputs[i], palindrome);
    }
  }

  if (failures != 0)
    return -1;

  libmin_success();
  return 0;
}
