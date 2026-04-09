/* Rabin-Karp rolling hash string search algorithm */

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

// D is the number of characters in the input alphabet 
#define D 256 

/* simple string search algorithm */
void
search(vector<VIP_ENCCHAR> txt, vector<VIP_ENCCHAR> pat, VIP_ENCBOOL *ret)
{
  int q = INT_MAX; 

  // value of h would be "pow(d, M-1)%q" 
  int h = 1;
  for (int i = 0; i < (int)pat.size() - 1; i++) 
    h = (h * D) % q; 

  VIP_ENCINT p = 0; // hash value for pattern 
  VIP_ENCINT t = 0; // hash value for txt 

  // calculate the hash value of pattern and first window of text 
  for (int i = 0; i < (int)pat.size(); i++) 
  { 
    p = (D * p + pat[i]) % q; 
    t = (D * t + txt[i]) % q; 
  } 

  // slide the pattern over text one by one 
  for (int i = 0; i <= (int)txt.size() - (int)pat.size(); i++) 
    { 
  
    // check the hash values of current window of text 
    // and pattern. If the hash values match then only 
    // check for characters one by one 
    ret[i] = (p == t);
  
    // calculate hash value for next window of text: Remove leading digit, add trailing digit 
#ifndef VIP_DO_MODE
    if (i < (int)txt.size()-(int)pat.size()) 
    { 
      t = (D*(t - ((VIP_ENCINT)txt[i])*h) + txt[i+pat.size()])%q; 
  
      // we might get negative value of t, converting it to positive 
      if (t < 0) 
        t = (t + q); 
    } 
#else /* VIP_DO_MODE */
    VIP_ENCBOOL _pred = (i < (int)txt.size()-(int)pat.size());
    t = VIP_CMOV(_pred, ((D*(t - ((VIP_ENCINT)txt[i])*h) + txt[i+pat.size()])%q), t);

    VIP_ENCBOOL _pred1 = (t < 0);
    t = VIP_CMOV(_pred && _pred1, t + q, t);
#endif /* VIP_DO_MODE */
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

