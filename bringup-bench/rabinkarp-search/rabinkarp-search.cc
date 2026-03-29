#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"
uint128_t simon_key = SIMON128_KEY;
simon_state_t simon_state;

typedef mojov_mem_fast_u64_t _uint64e_t;
typedef mojov_mem_fast_fp64_t _fp64e_t;
#include "mojov-exo.h"

// valid SIZE's: 380, 760 (default), 1140, 1520
#ifndef INPUT_SIZE
#define INPUT_SIZE 1520
#endif /* INPUT_SIZE */

// Inputs
const char inp_pat[] = "NzZVOzZXNoGXMl8yxesyY";
const char inp_txt[] =
  "zJfMus2WzLnMr82T4bmuzKTNjcylzYfNiGjMssyBZc2PzZPMvMyXzJnMvMyjzZQgzYfMnMyxzKDN"
  "k82NzZVOzZXNoGXMl8yxesyYzJ3MnMy6zZlwzKTMusy5zY3Mr82aZcygzLvMoM2ccsyozKTNjcy6"
  "zJbNlMyWzJZkzKDMn8ytzKzMnc2facymzZbMqc2TzZTMpGHMoMyXzKzNicyZbs2azZwgzLvMnsyw"
  "zZrNhWjMtc2JacyzzJ52zKLNh+G4mc2OzZ8t0onMrcypzLzNlG3MpMytzKtpzZXNh8ydzKZuzJfN"
  "meG4jcyfIMyvzLLNlc2ex6vMn8yvzLDMss2ZzLvMnWYgzKrMsMywzJfMlsytzJjNmGPMps2NzLLM"
#if INPUT_SIZE >= 760
  "ns2NzKnMmeG4pc2aYcyuzY7Mn8yZzZzGocypzLnNjnPMpC7MncydINKJWsyhzJbMnM2WzLDMo82J"
  "zJxhzqwerty42ZbMsM2ZzKzNoWzMssyrzLPNjcypZ8yhzJ/MvMyxzZrMnsyszYVvzJfNnC7Mnw=="
  "zKZIzKzMpMyXzKTNnWXNnCDMnMylzJ3Mu82NzJ/MgXfMlWjMlsyvzZNvzJ3NmcyWzY7MscyuINKJ"
  "zLrMmcyezJNzZVOzZXNoGXMl8yxesyY/NiFfiM2VzK3NmcyvzJx0zLbMvMyuc8yYzZnNlsyVIMyg"
  "nMyWIMywzYnMqc2HzZnMss2ezYVUzZbMvM2TzKrNomjNj82TzK7Mu2XMrMydzJ/NhSDMpMy5zJ1X"
#endif /* INPUT_SIZE >= 760 */
#if INPUT_SIZE >= 1140
  "ns2NzKnMmeG4pc2aYcyuzY7Mn8yZzZzGocypzLnNjnPMpC7MncydINKJWsyhzJbMnM2WzLDMo82J"
  "zJxhzqwerty42ZbMsM2ZzKzNoWzMssyrzLPNjcypZ8yhzJ/MvMyxzZrMnsyszYVvzJfNnC7Mnw=="
  "zKZIzKzMpMyXzKTNnWXNnCDMnMylzJ3Mu82NzJ/MgXfNzZVOzZXNoGXMl8yxesyYMlWjMlsyvNKJ"
  "zLrMmcyezJ/NiFfMt8y8zK1hzLrMqs2NxK/NiM2VzK3NmcyvzJx0zLbMvMyuc8yYzZnNlsyVIMyg"
  "nMyWIMywzYnMqc2HzZnMss2ezYVUzZbMvM2TzKrNomjNj82TzK7Mu2XMrMydzJ/NhSDMpMy5zJ1X"
#endif /* INPUT_SIZE >= 1140 */
#if INPUT_SIZE >= 1520
  "ns2NzKnMmeG4pc2aYcyuzY7Mn8yZNzZVOzZXNoGXMl8yxesyYzZzGNKJWsyhzJbMnM2WzLDMo82J"
  "zJxhzqwerty42ZbMsM2ZzKzNoWzMssyrzLPNjcypZ8yhzJ/MvMyxzZrMnsyszYVvzJfNnC7Mnw=="
  "zKZIzKzMpMyXzKTNnWXNnCDMnMylzJ3Mu82NzJ/MgXfMlWjMlsyvzZNvzJ3NmcyWzY7MscyuINKJ"
  "zLrMmcyezJ/NiFfMt8y8zK1hzLrMqs2NxK/NiM2VzK3NmcyvzJx0zLbMvMyuc8yYzZnNlsyVIMyg"
  "nMyWIMywzYnMqc2HzZnMss2ezYVUzZbMvM2TzKrNomjNj82TzK7Mu2XMrMydzJ/NhSDMpMy5zJ1X"
#endif /* INPUT_SIZE >= 1520 */
  ;

#define RK_BASE 0x9e3779b185ebca87ULL   // large odd constant

// Simple string search algorithm
// Fills res[0..text_len-1] with 0/1.
// res[i] == 1 means pattern starts at text[i].
// Returns the total number of matches found.
uint64e_t
rabinkarp_search(
    const uint64e_t *text,
    uint64_t text_len,
    const uint64e_t *pattern,
    uint64_t pattern_len,
    uint64e_t res[])
{
  if (text == NULL || pattern == NULL || res == NULL)
    return 0;

  // Clear result array
  for (size_t i = 0; i < text_len; i++)
        res[i] = 0;

  // Define empty-pattern behavior as "no matches"
  // because res has size text_len, not text_len + 1.
  if (pattern_len == 0)
    return 0;

  if (pattern_len > text_len)
    return 0;

  uint64e_t pattern_hash = 0;
  uint64e_t window_hash = 0;
  uint64e_t high_pow = 1;   // RK_BASE^(pattern_len - 1) mod 2^64

  // Compute RK_BASE^(pattern_len - 1)
  for (size_t i = 1; i < pattern_len; i++)
      high_pow *= RK_BASE;

  // Compute initial hashes
  for (size_t i = 0; i < pattern_len; i++)
  {
    pattern_hash = pattern_hash * RK_BASE + /*(unsigned char)*/pattern[i];
    window_hash  = window_hash  * RK_BASE + /*(unsigned char)*/text[i];
  }

  uint64e_t match_count = 0;

  for (size_t i = 0; i <= text_len - pattern_len; i++)
  {
    // Match found?
    uint64e_t found = (pattern_hash == window_hash);
    res[i] = cmov(found, 1lu, 0lu);
    match_count = cmov(found, match_count + 1, match_count);

    if (i < (text_len - pattern_len))
    {
      // Remove outgoing byte, shift, add incoming byte
      window_hash -= text[i] * high_pow;
      window_hash = window_hash * RK_BASE + text[i + pattern_len];
    }
  }

  return match_count;
}

int
main(void)
{
  if (mojov_configure_kmsm_from_dc_fast() != 0)
    return -1;

  // initilize cipher engine, for checking results
  simon_128_128_keyexpand(&simon_state, simon_key, 68);

  //
  // mprivregcfg tests
  //
  libmin_printf("** Running CSR[privreg] tests...\n");

  uint64_t val;

  // read reset value
  val = mojov_read_mprivregcfg();
  libmin_printf("Initial mprivregcfg = 0x%lx, ", val);
  mojov_print_mprivregcfg(val);
  libmin_printf("\n");

  // enable private register semantics (bit 0 = 1)
  if (mojov_enable_and_verify() != 0)
    return -1;

  val = mojov_read_mprivregcfg();
  libmin_printf("After enable, mprivregcfg = 0x%lx, ", val);
  mojov_print_mprivregcfg(val);
  libmin_printf("\n");

  // initialize the pseudo-RNG
  libmin_srand(42);

  int txt_len = libmin_strlen(inp_txt); // String lengths are public
  int pat_len = libmin_strlen(inp_pat); // String lengths are public
  libmin_printf("Input data: txt_len = %d, pat_len = %d\n", txt_len, pat_len);
  
  uint64e_t txt[txt_len];
  for (int k=0; k < txt_len; k++)
    txt[k] = inp_txt[k];

  uint64e_t pat[pat_len];
  for (int k=0; k < pat_len; k++)
    pat[k] = inp_pat[k];

  // Return vector
  uint64e_t ret[txt_len];
  for(int i=0; i<txt_len; i++)
    ret[i] = /* false */0; 

  uint64e_t matches;
  // Run search
  {
    // Stopwatch s("VIP_Bench Runtime");

    matches = rabinkarp_search(txt, txt_len, pat, pat_len, ret);
  }

  // print results
  libmin_printf("-- %lu matches detected --\n", mojov_decrypt_fast_u64(&simon_state, matches, CONTRACT_SIG));
  for(int i=0; i<txt_len; i++)
  {
    if (mojov_decrypt_fast_u64(&simon_state, ret[i], CONTRACT_SIG) != 0)
    { 
      libmin_printf("pattern detected at txt[%4d]\n", i);
    }
  }

  libmin_success();
  return 0;
}

