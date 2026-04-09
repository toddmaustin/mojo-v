/* C Program for Bad Character Heuristic of Boyer
Moore String Matching Algorithm */
// SOURCE: https://www.geeksforgeeks.org/boyer-moore-algorithm-for-pattern-searching/

#include <limits.h>
#include <string.h>
#include <stdio.h>
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

// The preprocessing function for Boyer Moore's
// bad character heuristic
void badCharHeuristic(vector<VIP_ENCCHAR> str, int size, VIP_ENCINT badchar[NO_OF_CHARS])
{
	// Initialize all occurrences as -1
	for(int i = 0; i < NO_OF_CHARS; i++)
  {
		badchar[i] = -1;
  }

	// Fill the actual value of last occurrence of a character
	for(int i = 0; i < size; i++)
  {
#ifndef VIP_DO_MEM
		badchar[(int)str[i]] = i;
#else
    VIP_ENCINT badchar_index = (VIP_ENCINT) str[i]; 
    // True data-obliviousness requires us to write to all indexes
    for(int j=0; j<NO_OF_CHARS; j++)
    {
      badchar[j] = VIP_CMOV(badchar_index==j, (VIP_ENCINT)j, badchar[j]);
    }
#endif
  }
}

/* A pattern searching function that uses Bad
Character Heuristic of Boyer Moore Algorithm */
void search(vector<VIP_ENCCHAR> txt, int n, vector<VIP_ENCCHAR> pat, int m, VIP_ENCBOOL *ret)
{
	VIP_ENCINT badchar[NO_OF_CHARS];

	/* Fill the bad character array by calling
	the preprocessing function badCharHeuristic()
	for given pattern */
	badCharHeuristic(pat, m, badchar);

	VIP_ENCINT s = 0; // s is shift of the pattern with
				     // respect to text
  
	for(int l=0; l<=(n - m); l++)
	{
#ifndef VIP_DO_IF
    //Early exit if not data-oblivious... 
    if(VIP_DEC(s) > (n-m)) break;
#endif 

		VIP_ENCINT idx = m-1;

		/* Keep reducing index idx of pattern while
		characters of pattern and text are
		matching at this shift s */
#ifndef VIP_DO_MODE
		while(idx >= 0 && pat[VIP_DEC(idx)] == txt[VIP_DEC(s+idx)])
    {
			idx--;
    }
#else
    for(int i=0; i<m; i++)
    {
       // Access: pat[VIP_DEC(idx)]
      VIP_ENCCHAR pat_idx = pat[0];
      for(int j=0; j<m; j++)
        pat_idx = VIP_CMOV(idx==j, pat[j], pat_idx);
      // Access: txt[VIP_DEC(s+idx)]
      VIP_ENCCHAR txt_idx = txt[0];
      for(int j=0; j<n; j++)
        txt_idx = VIP_CMOV(s+idx==j, txt[j], txt_idx);

      idx = idx - VIP_CMOV(idx >= 0 && pat_idx == txt_idx, (VIP_ENCINT)1,  (VIP_ENCINT)0);
    }
#endif

		/* If the pattern is present at current
		shift, then index idx will become -1 after
		the above loop */
    VIP_ENCBOOL cond = (idx < 0);
#ifndef VIP_DO_MODE
    if(cond)
    {
      ret[s] = true; 
			s += (s+m < n)? m-badchar[(int)txt[s+m]] : 1;
    }
		else 
    {
      VIP_ENCINT s_shift = idx - badchar[(int)txt[VIP_DEC(s+idx)]];
			s += 1 > s_shift ? 1 : s_shift;
    }
#else
    // True data-obliviousness requires us to write to all indexes
    for(int i=0; i<n; i++)
    {
      ret[i] = VIP_CMOV(cond && s==i, (VIP_ENCBOOL)true, ret[i]);
    }

    // Access: badchar[VIP_DEC(txt[VIP_DEC(s+m)])]
    VIP_ENCINT txt_sm = (VIP_ENCINT) txt[0];
    VIP_ENCINT badchar_txt_sm = badchar[0];
    for(int j=0; j<n; j++)
      txt_sm = VIP_CMOV(s+m==j, (VIP_ENCINT)txt[j], txt_sm);
    for(int j=0; j<NO_OF_CHARS; j++)
      badchar_txt_sm = VIP_CMOV(j==txt_sm, badchar[j], badchar_txt_sm);

    // Access: badchar[VIP_DEC(txt[VIP_DEC(s+idx)])]
    VIP_ENCINT txt_sidx = (VIP_ENCINT) txt[0];
    VIP_ENCINT badchar_txt_sidx = badchar[0];
    for(int j=0; j<n; j++)
      txt_sidx = VIP_CMOV(s+idx==j, (VIP_ENCINT)txt[j], txt_sidx);
    for(int j=0; j<NO_OF_CHARS; j++)
      badchar_txt_sidx = VIP_CMOV(j==txt_sidx, badchar[j], badchar_txt_sidx);

    VIP_ENCINT s_if   = VIP_CMOV(s+m < n, m-badchar_txt_sm, (VIP_ENCINT)1);
    VIP_ENCINT s_shift = idx - badchar_txt_sidx;
    VIP_ENCINT s_else =  VIP_CMOV((VIP_ENCINT)1 > s_shift, (VIP_ENCINT)1, s_shift);
    s = s + VIP_CMOV(cond, s_if, s_else);
#endif
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

    search(txt, n, pat, m, ret);
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

