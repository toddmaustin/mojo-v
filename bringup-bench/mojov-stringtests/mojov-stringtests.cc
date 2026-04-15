#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"
#include "mojov-string.h"

using namespace exo;

static int g_failures = 0;
static unsigned g_checks = 0;

static void check_bool(const char *label, bool ok)
{
  g_checks++;
  if (!ok)
  {
    g_failures++;
    libmin_printf("STRING-TEST: FAIL[%u] %s\n", g_checks, label);
  }
}

static uint64_t u64(uint64e_t v) { return v.decrypt(); }
static int64_t i64(int64e_t v) { return v.decrypt(); }
static uint8_t u8(uint8e_t v) { return v.decrypt(); }

static stringe_t make_string(const char *s, size_t len, size_t cap)
{
  stringe_t out(cap);
  for (size_t i = 0; i < len; ++i)
    out.push_back(uint8e_t((uint8_t)s[i]));
  return out;
}

int main(void)
{
  if (mojov_configure_kmsm_from_dc_fast() != 0) return -1;
  if (mojov_enable_and_verify() != 0) return -1;
  if (debug_context(SIMON128_KEY, CONTRACT_SIG) != 0) return -1;

  libmin_printf("STRING-TEST: START mojov-stringtests\n");

  // length/size/empty and packed indexing (crosses 8-byte packing boundary)
  stringe_t s = make_string("hello_world", 11, 16);
  check_bool("length()", s.length() == 16);
  check_bool("size()", u64(s.size()) == 11);
  check_bool("empty()==false", u64(s.empty()) == 0);
  check_bool("operator[] idx0", u8(s[0]) == (uint8_t)'h');
  check_bool("operator[] idx8", u8(s[8]) == (uint8_t)'r');

  // push_back / overflow -> exception sticky
  stringe_t tiny = make_string("abc", 3, 3);
  tiny.push_back(uint8e_t('z'));
  check_bool("push_back overflow sets exception", u64(tiny.exception()) != 0);
  check_bool("overflow keeps size bounded", u64(tiny.size()) == 3);

  // assignment semantics (reuse/effective copy for same len, realloc for different len)
  stringe_t a = make_string("abc", 3, 8);
  stringe_t b = make_string("xyz", 3, 8);
  b = a;
  check_bool("assign same length content", u8(b[1]) == (uint8_t)'b');

  stringe_t c = make_string("0123456789", 10, 10);
  c = a;
  check_bool("assign different length changed length", c.length() == 8);
  check_bool("assign different length content", u8(c[2]) == (uint8_t)'c');

  // append/operator+=/operator+
  stringe_t x = make_string("hel", 3, 12);
  x.push_back(uint8e_t('l'));
  x.append(uint8e_t('o'));
  x += uint8e_t('_');
  x += make_string("mojo", 4, 8);
  check_bool("append chain size", u64(x.size()) == 10);
  check_bool("append chain idx5", u8(x[5]) == (uint8_t)'m');

  stringe_t sum = make_string("ab", 2, 4) + make_string("cd", 2, 4);
  check_bool("operator+ size", u64(sum.size()) == 4);
  check_bool("operator+ content", u8(sum[3]) == (uint8_t)'d');

  // compare / spaceship / relational
  stringe_t ca = make_string("alpha", 5, 8);
  stringe_t cb = make_string("alphabet", 8, 12);
  stringe_t cc = make_string("alpha", 5, 8);
  check_bool("compare less", i64(ca.compare(cb)) < 0);
  check_bool("compare equal", i64(ca.compare(cc)) == 0);
  check_bool("operator<", u64(ca < cb) == 1);
  check_bool("operator==", u64(ca == cc) == 1);
  check_bool("operator>=", u64(cb >= ca) == 1);
  check_bool("operator<=>", i64(ca <=> cb) < 0);

  // find/rfind and variants
  stringe_t txt = make_string("bananaband", 9, 16);
  stringe_t needle = make_string("ana", 3, 8);
  check_bool("find substring", u64(txt.find(needle)) == 1);
  check_bool("rfind substring", u64(txt.rfind(needle)) == 3);
  check_bool("find char", u64(txt.find(uint8e_t('b'))) == 0);
  check_bool("rfind char", u64(txt.rfind(uint8e_t('b'))) == 6);

  stringe_t vowels = make_string("aeiou", 5, 8);
  check_bool("find_first_of", u64(txt.find_first_of(vowels)) == 1);
  check_bool("find_last_of", u64(txt.find_last_of(vowels)) == 5);
  check_bool("find_first_not_of", u64(txt.find_first_not_of(vowels)) == 0);
  check_bool("find_last_not_of", u64(txt.find_last_not_of(vowels)) == 8);
  check_bool("find_first_of(char)", u64(txt.find_first_of(uint8e_t('n'))) == 2);
  check_bool("find_last_of(char)", u64(txt.find_last_of(uint8e_t('a'))) == 5);
  check_bool("find_first_not_of(char)", u64(txt.find_first_not_of(uint8e_t('b'))) == 1);
  check_bool("find_last_not_of(char)", u64(txt.find_last_not_of(uint8e_t('d'))) == 7);

  // substr / starts_with / ends_with / contains
  stringe_t sub = txt.substr(uint64e_t(2), 4);
  check_bool("substr size", u64(sub.size()) == 4);
  check_bool("substr[0]", u8(sub[0]) == (uint8_t)'n');
  check_bool("starts_with", u64(txt.starts_with(make_string("ban", 3, 8))) == 1);
  check_bool("ends_with", u64(txt.ends_with(make_string("band", 4, 8))) == 1);
  check_bool("contains true", u64(txt.contains(make_string("nab", 3, 8))) == 1);
  check_bool("contains false", u64(txt.contains(make_string("zzz", 3, 8))) == 0);

  if (g_failures != 0)
  {
    libmin_printf("STRING-TEST: FAILURES %d/%u\n", g_failures, g_checks);
    return -1;
  }

  libmin_printf("STRING-TEST: PASS %u checks\n", g_checks);
  libmin_success();
  return 0;
}
