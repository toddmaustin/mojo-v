#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP32E_STORAGE_TYPE mojov_mem_fast_fp32_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"

typedef int8e_t VIP_ENCCHAR;
typedef uint64e_t VIP_ENCBOOL;
typedef uint64e_t VIP_ENCUINT64;
typedef fp32e_t VIP_ENCFLOAT;
typedef fp64e_t VIP_ENCDOUBLE;

typedef VIP_ENCCHAR sndex_t[4];

#define N_NAMES 24

static const char *namesA[N_NAMES] = {
  "Johnson",   "Adams",   "Davis",    "Simons",   "Richards", "Taylor",
  "Carter",    "Stevenson", "Taylor",   "Smith",    "McDonald", "Harris",
  "Sim",       "Williams", "Baker",    "Wells",    "Fraser",   "Jones",
  "Wilks",     "Hunt",    "Sanders",  "Parsons",  "Robson",   "Harker"
};

static const char *namesB[N_NAMES] = {
  "Jonson",    "Addams",  "Davies",   "Simmons",  "Richardson", "Tailor",
  "Chater",    "Stephenson", "Naylor",   "Smythe",   "MacDonald", "Harrys",
  "Sym",       "Wilson",  "Barker",   "Wills",    "Frazer",   "Johns",
  "Wilkinson", "Hunter",  "Saunders", "Pearson",  "Robertson", "Parker"
};

static int
ascii_upper(int c)
{
  if (c >= 'a' && c <= 'z')
    return c - 'a' + 'A';
  return c;
}

static void
string_to_soundex(const char *name, sndex_t sndex)
{
  unsigned si = 1;
  int c;

  static const char mappings[] = "01230120022455012623010202";

  sndex[0] = (char)ascii_upper((int)name[0]);

  for (unsigned i = 1; name[i] != '\0'; i++)
  {
    c = ascii_upper((int)name[i]) - 65;

    if (c >= 0 && c <= 25)
    {
      if (mappings[c] != '0')
      {
        if (mappings[c] != sndex[si - 1].decrypt())
        {
          sndex[si] = mappings[c];
          si++;
        }
        if (si > 3)
          break;
      }
    }
  }

  while (si <= 3)
  {
    sndex[si] = '0';
    si++;
  }
}

static VIP_ENCBOOL
soundex_equal(const sndex_t sndexA, const sndex_t sndexB)
{
  VIP_ENCBOOL equiv = true;

  for (int i = 0; i < 4; i++)
    equiv = cmov(sndexA[i] != sndexB[i], (VIP_ENCBOOL)false, equiv);

  return equiv;
}

int
main(void)
{
  if (mojov_configure_kmsm_from_dc_fast() != 0)
    return -1;

  // enable private register semantics (bit 0 = 1)
  if (mojov_enable_and_verify() != 0)
    return -1;

  // enable encrypted variable debugging
  if (debug_context(SIMON128_KEY, CONTRACT_SIG) != 0)
    return -1;

  // initialize the pseudo-RNG
  libmin_srand(42);

  sndex_t sndexA[N_NAMES];
  sndex_t sndexB[N_NAMES];
  VIP_ENCBOOL results[N_NAMES];

  for (unsigned i = 0; i < N_NAMES; i++)
  {
    string_to_soundex(namesA[i], sndexA[i]);
    string_to_soundex(namesB[i], sndexB[i]);
  }

  for (unsigned i = 0; i < N_NAMES; i++)
    results[i] = soundex_equal(sndexA[i], sndexB[i]);

  for (unsigned i = 0; i < N_NAMES; i++)
  {
    libmin_printf(
      "trial %3u: %-20s[%c%c%c%c] =? %-20s[%c%c%c%c] --> %s\n",
      i,
      namesA[i],
      sndexA[i][0].decrypt(), sndexA[i][1].decrypt(), sndexA[i][2].decrypt(), sndexA[i][3].decrypt(),
      namesB[i],
      sndexB[i][0].decrypt(), sndexB[i][1].decrypt(), sndexB[i][2].decrypt(), sndexB[i][3].decrypt(),
      results[i].decrypt() ? "true" : "false");
  }

  libmin_success();
  return 0;
}
