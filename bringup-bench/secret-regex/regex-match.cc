#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"
#include "mojov-string.h"

using namespace exo;

static constexpr unsigned kMaxStates = 5;
static constexpr unsigned kAlphabetClasses = 4; // {'a','b','c',other}
static constexpr unsigned kMaxTextLen = 16;
static constexpr unsigned kNumAutomata = 4;
static constexpr unsigned kNumTexts = 25;

struct RegexAutomaton
{
  const char *name;
  unsigned states;
  unsigned start_state;
  unsigned accept_state;
  unsigned trans[kMaxStates][kAlphabetClasses];
};

static const RegexAutomaton kAutomata[kNumAutomata] = {
  {
    "ab*c",
    4,
    0,
    3,
    {
      {1, 2, 2, 2}, // 0
      {2, 1, 3, 2}, // 1
      {2, 2, 2, 2}, // 2 sink
      {2, 2, 2, 2}, // 3 accept -> consume more => reject
      {0, 0, 0, 0}
    }
  },
  {
    "a.c",
    5,
    0,
    3,
    {
      {1, 4, 4, 4}, // 0
      {2, 2, 2, 2}, // 1 (dot: any char)
      {4, 4, 3, 4}, // 2
      {4, 4, 4, 4}, // 3 accept -> consume more => reject
      {4, 4, 4, 4}  // 4 sink
    }
  },
  {
    "ab+c",
    5,
    0,
    4,
    {
      {1, 4, 4, 4}, // 0
      {4, 2, 4, 4}, // 1
      {4, 2, 3, 4}, // 2
      {4, 4, 4, 4}, // 3 accept -> consume more => reject
      {4, 4, 4, 4}  // 4 sink
    }
  },
  {
    "a[^abc]*c",
    4,
    0,
    3,
    {
      {1, 2, 2, 2}, // 0
      {2, 2, 3, 1}, // 1
      {2, 2, 2, 2}, // 2 sink
      {2, 2, 2, 2}, // 3 accept -> consume more => reject
      {0, 0, 0, 0}
    }
  }
};

static const char *kPlainTexts[kNumTexts] = {
  "ac",
  "abc",
  "abbbc",
  "axc",
  "abbc",
  "abx",
  "abbbbc",
  "abbbcdef",
  "abbbc!",
  "abbbc?",
  "a12c",
  "a!@#$%^&*()_+c",
  "a!b!c",
  "acc",
  "a",
  "c",
  "bc",
  "cab",
  "bbb",
  "cccc",
  "baac",
  "ab",
  "acb",
  "a_cx",
  "zzzz"
};

static stringe_t encrypt_fixed_text(const char *s)
{
  stringe_t out(kMaxTextLen);
  for (unsigned i = 0; s[i] != '\0' && i < kMaxTextLen; ++i)
    out.push_back(uint8e_t((uint8_t)s[i]));
  return out;
}

static void secret_classify_char(uint8e_t ch, uint64e_t &is_a, uint64e_t &is_b, uint64e_t &is_c, uint64e_t &is_other)
{
  is_a = (uint64e_t)(ch == uint8e_t('a'));
  is_b = (uint64e_t)(ch == uint8e_t('b'));
  is_c = (uint64e_t)(ch == uint8e_t('c'));
  is_other = !(is_a || is_b || is_c);
}

static uint64e_t secret_match(const RegexAutomaton &aut, const stringe_t &text)
{
  uint64e_t active[kMaxStates];
  uint64e_t next_active[kMaxStates];

  for (unsigned s = 0; s < kMaxStates; ++s)
    active[s] = uint64e_t(s == aut.start_state ? 1 : 0);

  for (uint64_t i = 0; i < kMaxTextLen; ++i)
  {
    uint64e_t process = uint64e_t(i) < text.size();

    uint8e_t ch(0);
    for (unsigned j = 0; j < kMaxTextLen; ++j)
    {
      uint64e_t pick = process && (uint64e_t(i == j));
      ch = cmov(pick, text[j], ch);
    }

    uint64e_t is_a, is_b, is_c, is_other;
    secret_classify_char(ch, is_a, is_b, is_c, is_other);

    for (unsigned s = 0; s < kMaxStates; ++s)
      next_active[s] = uint64e_t(0);

    for (unsigned t = 0; t < aut.states; ++t)
    {
      for (unsigned s = 0; s < aut.states; ++s)
      {
        uint64e_t on_a = uint64e_t(aut.trans[t][0] == s);
        uint64e_t on_b = uint64e_t(aut.trans[t][1] == s);
        uint64e_t on_c = uint64e_t(aut.trans[t][2] == s);
        uint64e_t on_o = uint64e_t(aut.trans[t][3] == s);

        uint64e_t edge = (is_a & on_a) | (is_b & on_b) | (is_c & on_c) | (is_other & on_o);
        next_active[s] = next_active[s] | (active[t] & edge);
      }
    }

    for (unsigned s = 0; s < kMaxStates; ++s)
      active[s] = cmov(process, next_active[s], active[s]);
  }

  return active[aut.accept_state];
}

int main(void)
{
  if (mojov_configure_kmsm_from_dc_fast() != 0)
    return -1;
  if (mojov_enable_and_verify() != 0)
    return -1;
  if (debug_context(SIMON128_KEY, CONTRACT_SIG) != 0)
    return -1;

  stringe_t texts[kNumTexts];
  for (unsigned i = 0; i < kNumTexts; ++i)
    texts[i] = encrypt_fixed_text(kPlainTexts[i]);

  for (unsigned a = 0; a < kNumAutomata; ++a)
  {
    uint64e_t matches(0);
    libmin_printf("regex `%s` matches:\n", kAutomata[a].name);

    for (unsigned i = 0; i < kNumTexts; ++i)
    {
      uint64e_t is_match = secret_match(kAutomata[a], texts[i]);
      matches = matches + is_match;

      if (is_match.decrypt() != 0)
        libmin_printf("  %s\n", kPlainTexts[i]);
    }

    libmin_printf("total matches: %lu\n\n", matches.decrypt());
  }

  libmin_success();
  return 0;
}
