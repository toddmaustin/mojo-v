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
#define MAX_TRANSFORMED_LEN (2 * MAX_STR_LEN + 1)

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

static uint8e_t
transformed_at(const stringe_t &s, uint64_t t_index)
{
  if ((t_index & 1u) == 0u)
    return uint8e_t('#');
  return s[t_index >> 1];
}

static PalResult
longest_palindrome_manacher_encrypted(const stringe_t &s, uint64_t n)
{
  const uint64_t t_len = 2 * n + 1;
  uint64e_t p[MAX_TRANSFORMED_LEN];

  for (uint64_t i = 0; i < t_len; ++i)
    p[i] = 0;

  uint64_t center = 0;
  uint64_t right = 0;
  uint64e_t best_len = 0;
  uint64e_t best_start = n;

  for (uint64_t i = 0; i < t_len; ++i)
  {
    if (i < right)
    {
      uint64_t mirror = (2 * center) - i;
      uint64_t mirror_plain = p[mirror].decrypt();
      uint64_t bound = right - i;
      p[i] = (mirror_plain < bound) ? uint64e_t(mirror_plain) : uint64e_t(bound);
    }

    while ((i > p[i].decrypt()) && ((i + p[i].decrypt() + 1) < t_len))
    {
      uint64_t left_t = i - p[i].decrypt() - 1;
      uint64_t right_t = i + p[i].decrypt() + 1;

      uint64e_t eq = transformed_at(s, left_t) == transformed_at(s, right_t);
      if (!eq.decrypt())
        break;

      p[i] = p[i] + 1;
    }

    uint64_t cur_rad = p[i].decrypt();
    if (i + cur_rad > right)
    {
      center = i;
      right = i + cur_rad;
    }

    uint64e_t cand_len = p[i];
    uint64e_t cand_start = uint64e_t((i - cur_rad) / 2);

    uint64e_t better = (cand_len > best_len) || ((cand_len == best_len) && (cand_start < best_start));
    best_len = cmov(better, cand_len, best_len);
    best_start = cmov(better, cand_start, best_start);
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
  uint64_t agg_start = 0;
  uint64_t agg_len = 0;

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

    libmin_printf("manacher-lps[%lu]: start=%lu len=%lu\n", i, got_start, got_len);

    agg_start += got_start;
    agg_len += got_len;

    if (got_start != exp_start || got_len != exp_len)
    {
      failures++;
      libmin_printf("ERROR: mismatch i=%lu expected=(%lu,%lu) got=(%lu,%lu)\n",
        i, exp_start, exp_len, got_start, got_len);
    }
  }

  libmin_printf("manacher-lps: aggregate start=%lu len=%lu\n", agg_start, agg_len);

  if (failures != 0)
    return -1;

  libmin_success();
  return 0;
}
