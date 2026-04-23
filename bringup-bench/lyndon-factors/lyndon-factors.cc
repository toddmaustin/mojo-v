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
#define MAX_FACTORS MAX_STR_LEN
#define N_STRINGS 8

struct FactorizationResult {
  uint64e_t starts[MAX_FACTORS];
  uint64e_t lens[MAX_FACTORS];
  uint64e_t count;
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
secret_char_at(const stringe_t &s, uint64e_t idx, uint64_t n)
{
  uint8e_t out(0);
  for (uint64_t p = 0; p < MAX_STR_LEN; ++p)
  {
    const uint64e_t in_range = uint64e_t(p) < uint64e_t(n);
    const uint64e_t choose = in_range && (idx == uint64e_t(p));
    out = cmov(choose, s[p], out);
  }
  return out;
}

static void
oblivious_store_u64e(uint64e_t arr[MAX_FACTORS], uint64e_t secret_idx, uint64e_t value)
{
  for (uint64_t slot = 0; slot < MAX_FACTORS; ++slot)
  {
    const uint64e_t choose = secret_idx == uint64e_t(slot);
    arr[slot] = cmov(choose, value, arr[slot]);
  }
}

static FactorizationResult
duval_factorization_encrypted_oblivious(const stringe_t &s, uint64_t n)
{
  FactorizationResult out;
  for (uint64_t i = 0; i < MAX_FACTORS; ++i)
  {
    out.starts[i] = 0;
    out.lens[i] = 0;
  }
  out.count = 0;

  uint64e_t i = 0;
  uint64e_t j = 1;
  uint64e_t k = 0;
  uint64e_t phase_emit = 0;
  uint64e_t emit_period = 1;
  uint64e_t emit_k_limit = 0;
  uint64e_t done = 0;

  const uint64_t max_steps = MAX_STR_LEN * MAX_STR_LEN * 4;

  for (uint64_t step = 0; step < max_steps; ++step)
  {
    const uint64e_t i_in = i < uint64e_t(n);
    const uint64e_t j_in = j < uint64e_t(n);
    const uint64e_t search_active = !done && !phase_emit && i_in;

    const uint8e_t ck = secret_char_at(s, k, n);
    const uint8e_t cj = secret_char_at(s, j, n);

    const uint64e_t lt = search_active && j_in && (uint64e_t)(ck < cj);
    const uint64e_t eq = search_active && j_in && (uint64e_t)(ck == cj);
    const uint64e_t gt = search_active && j_in && (uint64e_t)(ck > cj);

    const uint64e_t step_period = j - k;

    const uint64e_t i_after_reset = i + step_period;
    const uint64e_t j_after_reset = i_after_reset + uint64e_t(1);

    i = cmov(gt, i_after_reset, i);
    j = cmov(gt, j_after_reset, j);
    k = cmov(gt, i_after_reset, k);

    const uint64e_t j_inc = j + uint64e_t(1);
    j = cmov(lt || eq, j_inc, j);
    k = cmov(lt, i, k);
    k = cmov(eq, k + uint64e_t(1), k);

    const uint64e_t emit_trigger = search_active && (gt || !j_in);
    phase_emit = cmov(emit_trigger, uint64e_t(1), phase_emit);
    emit_period = cmov(emit_trigger, step_period, emit_period);
    emit_k_limit = cmov(emit_trigger, k, emit_k_limit);

    const uint64e_t emit_active = !done && phase_emit && i_in && (i <= emit_k_limit);
    oblivious_store_u64e(out.starts, out.count, i);
    oblivious_store_u64e(out.lens, out.count, emit_period);
    out.count = cmov(emit_active, out.count + uint64e_t(1), out.count);
    i = cmov(emit_active, i + emit_period, i);

    const uint64e_t emit_finished = phase_emit && (!i_in || (i > emit_k_limit));
    phase_emit = cmov(emit_finished, uint64e_t(0), phase_emit);
    j = cmov(emit_finished, i + uint64e_t(1), j);
    k = cmov(emit_finished, i, k);

    done = done || (i >= uint64e_t(n));
  }

  return out;
}

static uint64_t
plain_duval(const char *s, uint64_t n, uint64_t starts[MAX_FACTORS], uint64_t lens[MAX_FACTORS])
{
  uint64_t factor_count = 0;
  uint64_t i = 0;
  while (i < n)
  {
    uint64_t j = i + 1;
    uint64_t k = i;

    while (j < n && s[k] <= s[j])
    {
      if (s[k] < s[j])
        k = i;
      else
        ++k;
      ++j;
    }

    uint64_t period = j - k;
    while (i <= k)
    {
      starts[factor_count] = i;
      lens[factor_count] = period;
      ++factor_count;
      i += period;
    }
  }
  return factor_count;
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
    "banana",
    "mississippi",
    "abcabca",
    "zzzzzz",
    "abababab",
    "cabbaabbac",
    "qwertyqwert",
    "lyndonlyndon"
  };

  uint64_t total_factors = 0;
  uint64_t total_len = 0;
  uint64_t failures = 0;

  libmin_printf("lyndon-factors: processing %d encrypted strings\n", N_STRINGS);

  for (uint64_t t = 0; t < N_STRINGS; ++t)
  {
    const uint64_t n = libmin_strlen(plain_inputs[t]);
    if (n > MAX_STR_LEN)
      return -1;

    stringe_t enc = make_encrypted_string(plain_inputs[t], n);
    FactorizationResult enc_res = duval_factorization_encrypted_oblivious(enc, n);

    uint64_t ref_starts[MAX_FACTORS];
    uint64_t ref_lens[MAX_FACTORS];
    for (uint64_t i = 0; i < MAX_FACTORS; ++i)
    {
      ref_starts[i] = 0;
      ref_lens[i] = 0;
    }
    const uint64_t ref_count = plain_duval(plain_inputs[t], n, ref_starts, ref_lens);

    const uint64_t got_count = enc_res.count.decrypt();
    libmin_printf("lyndon[%lu]: \"%s\" => factors=%lu\n", t, plain_inputs[t], got_count);

    if (got_count != ref_count)
      failures++;

    uint64_t recovered_total = 0;
    for (uint64_t f = 0; f < got_count; ++f)
    {
      uint64_t got_start = enc_res.starts[f].decrypt();
      uint64_t got_len = enc_res.lens[f].decrypt();
      recovered_total += got_len;

      libmin_printf("  factor[%2lu]: start=%2lu len=%2lu text=\"", f, got_start, got_len);
      for (uint64_t c = 0; c < got_len; ++c)
        libmin_printf("%c", plain_inputs[t][got_start + c]);
      libmin_printf("\"\n");

      if (f >= ref_count || got_start != ref_starts[f] || got_len != ref_lens[f])
        failures++;
    }

    if (recovered_total != n)
      failures++;

    total_factors += got_count;
    total_len += recovered_total;
  }

  libmin_printf("lyndon totals: factor_count_sum=%lu recovered_len_sum=%lu\n", total_factors, total_len);

  if (failures != 0)
  {
    libmin_printf("lyndon FAIL: mismatches=%lu\n", failures);
    return -1;
  }

  libmin_printf("lyndon PASS: encrypted Duval-style factorization matched plaintext reference\n");
  libmin_success();
  return 0;
}
