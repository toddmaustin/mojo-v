#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"

// valid INPUT_SIZE values: 380, 760 (default), 1140, 1520
#ifndef INPUT_SIZE
#define INPUT_SIZE 760
#endif /* INPUT_SIZE */

static const char inp_pat[] = "NzZVOzZXNoGXMl8yxesyY";
static const char inp_txt[] =
  "zJfMus2WzLnMr82T4bmuzKTNjcylzYfNiGjMssyBZc2PzZPMvMyXzJnMvMyjzZQgzYfMnMyxzKDN"
  "k82NzZVOzZXNoGXMl8yxesyYzJ3MnMy6zZlwzKTMusy5zY3Mr82aZcygzLvMoM2ccsyozKTNjcy6"
  "zJbNlMyWzJZkzKDMn8ytzKzMnc2facymzZbMqc2TzZTMpGHMoMyXzKzNicyZbs2azZwgzLvMnsyw"
  "zZrNhWjMtc2JacyzzJ52zKLNh+G4mc2OzZ8t0onMrcypzLzNlG3MpMytzKtpzZXNh8ydzKZuzJfN"
  "meG4jcyfIMyvzLLNlc2ex6vMn8yvzLDMss2ZzLvMnWYgzKrMsMywzJfMlsytzJjNmGPMps2NzLLM"
#if INPUT_SIZE >= 760
  "ns2NzKnMmeG4pc2aYcyuzY7Mn8yZzZzGocypzLnNjnPMpC7MncydINKJWsyhzJbMnM2WzLDMo82J"
  "zJxhzqwerty42ZbMsM2ZzKzNoWzMssyrzLPNjcypZ8yhzJ/MvMyxzZrMnsyszYVvzJfNnC7Mnw=="
  "zKZIzKzMpMyXzKTNnWXNnCDMnMylzJ3Mu82NzJ/MgXfMlWjMlsyvzZNvzJ3NmcyWzY7MscyuINKJ"
  "zLrMmcyezJ/NiFfMt8y8zK1hzLrMqs2NxK/NiM2VzK3NmcyvzJx0zLbMvMyuc8yYzZnNlsyVIMyg"
  "nMyWIMywzYnMqc2HzZnMss2ezYVUzZbMvM2TzKrNomjNj82TzK7Mu2XMrMydzJ/NhSDMpMy5zJ1X"
#endif /* INPUT_SIZE >= 760 */
#if INPUT_SIZE >= 1140
  "ns2NzKnMmeG4pc2aYcyuzY7Mn8yZzZzGocypzLnNjnPMpC7MncydINKJWsyhzJbMnM2WzLDMo82J"
  "zJxhzqwerty42ZbMsM2ZzKzNoWzMssyrzLPNjcypZ8yhzJ/MvMyxzZrMnsyszYVvzJfNnC7Mnw=="
  "zKZIzKzMpMyXzKTNnWXNnCDMnMylzJ3Mu82NzJ/MgXfMlWjMlsyvzZNvzJ3NmcyWzY7MscyuINKJ"
  "zLrMmcyezJ/NiFfMt8y8zK1hzLrMqs2NxK/NiM2VzK3NmcyvzJx0zLbMvMyuc8yYzZnNlsyVIMyg"
  "nMyWIMywzYnMqc2HzZnMss2ezYVUzZbMvM2TzKrNomjNj82TzK7Mu2XMrMydzJ/NhSDMpMy5zJ1X"
#endif /* INPUT_SIZE >= 1140 */
#if INPUT_SIZE >= 1520
  "ns2NzKnMmeG4pc2aYcyuzY7Mn8yZzZzGocypzLnNjnPMpC7MncydINKJWsyhzJbMnM2WzLDMo82J"
  "zJxhzqwerty42ZbMsM2ZzKzNoWzMssyrzLPNjcypZ8yhzJ/MvMyxzZrMnsyszYVvzJfNnC7Mnw=="
  "zKZIzKzMpMyXzKTNnWXNnCDMnMylzJ3Mu82NzJ/MgXfMlWjMlsyvzZNvzJ3NmcyWzY7MscyuINKJ"
  "zLrMmcyezJ/NiFfMt8y8zK1hzLrMqs2NxK/NiM2VzK3NmcyvzJx0zLbMvMyuc8yYzZnNlsyVIMyg"
  "nMyWIMywzYnMqc2HzZnMss2ezYVUzZbMvM2TzKrNomjNj82TzK7Mu2XMrMydzJ/NhSDMpMy5zJ1X"
#endif /* INPUT_SIZE >= 1520 */
  ;

// VIP_ENCCHAR => int8e_t
// VIP_ENCBOOL => uint64e_t
// VIP_ENCUINT64 => uint64e_t
// VIP_ENCFLOAT => fp32e_t
// VIP_ENCDOUBLE => fp64e_t

static uint64e_t
search(const int8e_t txt[], uint64_t txt_len, const int8e_t pat[], uint64_t pat_len, uint64e_t ret[])
{
  for (uint64_t i = 0; i < txt_len; i++)
    ret[i] = 0;

  if (pat_len == 0 || pat_len > txt_len)
    return 0;

  uint64e_t match_count = 0;
  uint64_t last = txt_len - pat_len;
  for (uint64_t i = 0; i <= last; i++)
  {
    uint64e_t match = 1;
    for (uint64_t j = 0; j < pat_len; j++)
    {
      uint64e_t pred = (txt[i + j] != pat[j]);
      match = cmov(match && pred, (uint64e_t)0, match);
    }

    ret[i] = match;
    match_count = cmov(match, match_count + 1, match_count);
  }

  return match_count;
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

  int txt_len = libmin_strlen(inp_txt); // String lengths are public
  int pat_len = libmin_strlen(inp_pat); // String lengths are public
  libmin_printf("n = %d, m = %d\n", txt_len, pat_len);

  int8e_t txt[txt_len];
  for (int k = 0; k < txt_len; k++)
    txt[k] = inp_txt[k];

  int8e_t pat[pat_len];
  for (int k = 0; k < pat_len; k++)
    pat[k] = inp_pat[k];

  uint64e_t ret[txt_len];
  uint64e_t matches = search(txt, txt_len, pat, pat_len, ret);

  libmin_printf("-- %lu matches detected --\n", matches.decrypt());
  for (int i = 0; i < txt_len; i++)
  {
    if (ret[i].decrypt() != 0)
      libmin_printf("pattern occurs at shift = %d\n", i);
  }

  libmin_success();
  return 0;
}
