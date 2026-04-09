//
// A Dynamic Programming based solution for 0-1 Knapsack problem
//
// The All ▲lgorithms library for python
//
// https://allalgorithms.com/dynamic-programming/
// https://github.com/allalgorithms/cpp
//
// Contributed by: Unknown
// Github: Unknown
//
#include <iostream>
using namespace std;

#include "../config.h"

#ifdef PROBLEM_TINY
#define N 3
#define W 50
VIP_ENCINT val[N] = {60, 100, 120};
VIP_ENCINT wt[N] = {10, 20, 30};
#else /* !PROBLEM_TINY */
#define N 50
#define W 250
VIP_ENCINT val[N] = { 27, 34, 9, 22, 8, 17, 22, 21, 23, 19, 7, 36, 11, 42, 37, 16, 10, 26, 10, 50, 23, 46, 37, 3, 14, 16, 35, 14, 15, 44, 49, 2, 45, 3, 15, 1, 34, 44, 19, 25, 43, 28, 26, 4, 30, 24, 49, 11, 48, 13 };
VIP_ENCINT wt[N] = { 23, 47, 22, 15, 42, 30, 15, 32, 47, 33, 15, 38, 44, 7, 16, 34, 30, 33, 3, 2, 43, 31, 46, 17, 30, 1, 34, 21, 30, 21, 29, 21, 36, 14, 18, 21, 13, 3, 27, 44, 33, 11, 9, 31, 40, 40, 30, 9, 41, 31 };
#endif /* !PROBLEM_TINY */


// A utility function that returns maximum of two integers
VIP_ENCINT
max(VIP_ENCINT a, VIP_ENCINT b)
{
#ifdef VIP_DO_MODE
  VIP_ENCINT retval = VIP_CMOV(a > b, a, b);
  return retval; 
#else /* !VIP_DO_MODE */
  if (a > b)
    return a;
  else
    return b;
#endif /* !VIP_DO_MODE */
}

// Returns the maximum value that can be put in a knapsack of capacity W
void
knapSack(VIP_ENCINT wt[], VIP_ENCINT val[], VIP_ENCINT K[N+1][W+1])
{
  int i, w;

  // Build table K[][] in bottom up manner
  for (i = 0; i <= N; i++)
  {
    for (w = 0; w <= W; w++)
    {
      if (i==0 || w==0)
        K[i][w] = 0;
      else
      {
#ifdef VIP_DO_MODE
        VIP_ENCINT Kw = -1;
        for (int k=0; k<=W; k++)
          Kw = VIP_CMOV((VIP_ENCINT)k == ((VIP_ENCINT)w-wt[i-1]), K[i-1][k], Kw);
        VIP_ENCINT maxval = max(val[i-1] + Kw,  K[i-1][w]);
        K[i][w] = VIP_CMOV(wt[i-1] <= w, maxval, K[i-1][w]);
#else /* !VIP_DO_MODE */
        if (wt[i-1] <= w)
          K[i][w] = max(val[i-1] + K[i-1][w-wt[i-1]],  K[i-1][w]);
        else
          K[i][w] = K[i-1][w];
#endif /* !VIP_DO_MODE */
      }
    }
  }
}

int
main(void)
{
  int n = N;
  int w = W;
  VIP_ENCINT K[N+1][W+1];

  {
    Stopwatch s("VIP-Bench knapsack:");

    knapSack(wt, val, K);
  }

	fprintf(stdout, "Max value: %d\n", VIP_DEC(K[n][W]));
	
  fprintf(stdout, "Selected packs:\n");
  while (n != 0)
  {
    if (VIP_DEC(K[n][w]) != VIP_DEC(K[n - 1][w])) {
      fprintf(stdout, "  Package %d with wt=%d and val=%d\n", n, VIP_DEC(wt[n - 1]), VIP_DEC(val[n - 1]));
      w = w - VIP_DEC(wt[n-1]);
    }
    n--;
  }
  fprintf(stdout, "Total weight: %d\n", W - w);

  return 0;
}

#ifdef notdef
  for (i=0; i <= n; i++)
  {
    for (w=0; w <= W; w++)
    {
      fprintf(stdout, "%3d ", K[i][w]);
    }
    fprintf(stdout, "\n");
  }
#endif /* notdef */
