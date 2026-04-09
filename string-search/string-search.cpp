/* Simple string search algorithm */

# include <limits.h>
# include <string.h>
# include <stdio.h>
#include "../config.h"

#ifdef VIP_DO_MODE
#define VIP_DO_IF
#define VIP_DO_LOOP
#define VIP_DO_MEM
#endif /* VIP_DO_MODE */

using namespace std;

// valid SIZE's: 380, 760 (default), 1140, 1520
#define SIZE 760

// Inputs
char inp_pat[] = "NzZVOzZXNoGXMl8yxesyY";
char inp_txt[] =
  "zJfMus2WzLnMr82T4bmuzKTNjcylzYfNiGjMssyBZc2PzZPMvMyXzJnMvMyjzZQgzYfMnMyxzKDN"
  "k82NzZVOzZXNoGXMl8yxesyYzJ3MnMy6zZlwzKTMusy5zY3Mr82aZcygzLvMoM2ccsyozKTNjcy6"
  "zJbNlMyWzJZkzKDMn8ytzKzMnc2facymzZbMqc2TzZTMpGHMoMyXzKzNicyZbs2azZwgzLvMnsyw"
  "zZrNhWjMtc2JacyzzJ52zKLNh+G4mc2OzZ8t0onMrcypzLzNlG3MpMytzKtpzZXNh8ydzKZuzJfN"
  "meG4jcyfIMyvzLLNlc2ex6vMn8yvzLDMss2ZzLvMnWYgzKrMsMywzJfMlsytzJjNmGPMps2NzLLM"
#if SIZE >= 760
  "ns2NzKnMmeG4pc2aYcyuzY7Mn8yZzZzGocypzLnNjnPMpC7MncydINKJWsyhzJbMnM2WzLDMo82J"
  "zJxhzqwerty42ZbMsM2ZzKzNoWzMssyrzLPNjcypZ8yhzJ/MvMyxzZrMnsyszYVvzJfNnC7Mnw=="
  "zKZIzKzMpMyXzKTNnWXNnCDMnMylzJ3Mu82NzJ/MgXfMlWjMlsyvzZNvzJ3NmcyWzY7MscyuINKJ"
  "zLrMmcyezJ/NiFfMt8y8zK1hzLrMqs2NxK/NiM2VzK3NmcyvzJx0zLbMvMyuc8yYzZnNlsyVIMyg"
  "nMyWIMywzYnMqc2HzZnMss2ezYVUzZbMvM2TzKrNomjNj82TzK7Mu2XMrMydzJ/NhSDMpMy5zJ1X"
#endif /* SIZE >= 760 */
#if SIZE >= 1140
  "ns2NzKnMmeG4pc2aYcyuzY7Mn8yZzZzGocypzLnNjnPMpC7MncydINKJWsyhzJbMnM2WzLDMo82J"
  "zJxhzqwerty42ZbMsM2ZzKzNoWzMssyrzLPNjcypZ8yhzJ/MvMyxzZrMnsyszYVvzJfNnC7Mnw=="
  "zKZIzKzMpMyXzKTNnWXNnCDMnMylzJ3Mu82NzJ/MgXfMlWjMlsyvzZNvzJ3NmcyWzY7MscyuINKJ"
  "zLrMmcyezJ/NiFfMt8y8zK1hzLrMqs2NxK/NiM2VzK3NmcyvzJx0zLbMvMyuc8yYzZnNlsyVIMyg"
  "nMyWIMywzYnMqc2HzZnMss2ezYVUzZbMvM2TzKrNomjNj82TzK7Mu2XMrMydzJ/NhSDMpMy5zJ1X"
#endif /* SIZE >= 1140 */
#if SIZE >= 1520
  "ns2NzKnMmeG4pc2aYcyuzY7Mn8yZzZzGocypzLnNjnPMpC7MncydINKJWsyhzJbMnM2WzLDMo82J"
  "zJxhzqwerty42ZbMsM2ZzKzNoWzMssyrzLPNjcypZ8yhzJ/MvMyxzZrMnsyszYVvzJfNnC7Mnw=="
  "zKZIzKzMpMyXzKTNnWXNnCDMnMylzJ3Mu82NzJ/MgXfMlWjMlsyvzZNvzJ3NmcyWzY7MscyuINKJ"
  "zLrMmcyezJ/NiFfMt8y8zK1hzLrMqs2NxK/NiM2VzK3NmcyvzJx0zLbMvMyuc8yYzZnNlsyVIMyg"
  "nMyWIMywzYnMqc2HzZnMss2ezYVUzZbMvM2TzKrNomjNj82TzK7Mu2XMrMydzJ/NhSDMpMy5zJ1X"
#endif /* SIZE >= 1520 */
  ;

#define NO_OF_CHARS 256

/* simple string search algorithm */
void
search(vector<VIP_ENCCHAR> txt, vector<VIP_ENCCHAR> pat, VIP_ENCBOOL *ret)
{
  for (unsigned i=0; i < txt.size(); i++)
  {
    VIP_ENCBOOL match = true;
    for (unsigned j=0; j < pat.size(); j++)
    {
#ifndef VIP_DO_MODE
      if (txt[i+j] != pat[j])
      {
        match = false;
        break;
      }
#else /* VIP_DO_MODE */
      VIP_ENCBOOL _pred = (txt[i+j] != pat[j]);
      match = VIP_CMOV(match && _pred, (VIP_ENCBOOL)false, match);
#endif /* VIP_DO_MODE */
    }
    ret[i] = match;
  }
}


int
main(void) 
{ 
  VIP_INIT; 

  int n = strlen(inp_txt); // String lengths are public
  int m = strlen(inp_pat); // String lengths are public
  printf("n = %d, m = %d\n", n, m);
  
  vector<VIP_ENCCHAR> txt(n);
  for (int k=0; k < n; k++)
    txt[k] = inp_txt[k];

  vector<VIP_ENCCHAR> pat(m);
  for (int k=0; k < m; k++)
    pat[k] = inp_pat[k];

  // Return vector
  VIP_ENCBOOL  ret[n];
  for(int i=0; i<n; i++) ret[i] = false; 
	

  // Run search
  {
    Stopwatch s("VIP_Bench Runtime");

    search(txt, pat, ret);
  }

  // print results
  for(int i=0; i<n; i++)
  {
    if(VIP_DEC(ret[i])) 
    { 
      printf("pattern occurs at shift = %d\n", i);
    }
  }

  return 0;
} 

