#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utils.h"

// Randomized Shell sort, from: https://arxiv.org/pdf/0909.1037.pdf

// include build configuration defines
#include "../config.h"

// supported sizes: 256, 512, 1024 (default), 2048
#define DATASET_SIZE 1024
VIP_ENCINT data[DATASET_SIZE];

// total swaps executed so far
unsigned long swaps = 0;

#define C 1

void
print_data(VIP_ENCINT *data, unsigned size)
{
  fprintf(stdout, "DATA DUMP:\n");
  for (unsigned i=0; i < size; i++)
    fprintf(stdout, "  data[%u] = %d\n", i, VIP_DEC(data[i]));
}

static inline void
exchange(int *a, int i, int j)
{
  int temp = a[i];
  a[i] = a[j];
  a[j] = temp;
  swaps++;
}

static inline void
compareExchange(VIP_ENCINT *a, int i, int j)
{
#ifndef VIP_DO_MODE
  if (((i < j) && (a[i] > a[j])) || ((i > j) && (a[i] < a[j])))
  {
    VIP_ENCINT temp = a[i];
    a[i] = a[j];
    a[j] = temp;
    swaps++;
  }
#else /* VIP_DO_MODE */
  VIP_ENCBOOL _pred = (((VIP_ENCBOOL)(i < j) && (a[i] > a[j])) || ((VIP_ENCBOOL)(i > j) && (a[i] < a[j])));
  VIP_ENCINT temp = a[i];
  a[i] = VIP_CMOV(_pred, a[j], a[i]);
  a[j] = VIP_CMOV(_pred, temp, a[j]);
  swaps++;
#endif /* VIP_DO_MODE */
}

void
permuteRandom(int *a, int size)
{
  for (int i=0; i < size; i++) // Use the Knuth random perm. algorithm
    exchange(a, i, myrand() % ((size-i)+i));
}
// compare-exchange two regions of length offset each
void
compareRegions(VIP_ENCINT *a, int size, int s, int t, int offset)
{
  int mate[offset]; // index offset array
  for (int count=0; count < C; count++) // do C region compare-exchanges 20
  {
    for (int i=0; i < offset; i++)
      mate[i] = i;
    permuteRandom(mate, offset); // comment this out to get a deterministic Shellsort
    for (int i=0; i < offset; i++)
      compareExchange(a, s+i, t+mate[i]);
  }
}

// given an array arr of length n, this code sorts it in place
// all indices run from 0 to n-1
void
randshellsort(VIP_ENCINT *data, unsigned size)
{
  int n = (int)size; // we assume that n is a power of 2

  for (int offset = n/2; offset > 0; offset /= 2)
  {
    for (int i=0; i < n - offset; i += offset) // compare-exchange up
      compareRegions(data, n, i, i+offset, offset);
    for (int i=n-offset; i >= offset; i -= offset) // compare-exchange down
      compareRegions(data, n, i-offset, i, offset);
    for (int i=0; i < n-3*offset; i += offset) // compare 3 hops up
      compareRegions(data, n, i, i+3*offset, offset);
    for (int i=0; i < n-2*offset; i += offset) // compare 2 hops up
      compareRegions(data, n, i, i+2*offset, offset);
    for (int i=0; i < n; i += 2*offset) // compare odd-even regions
      compareRegions(data, n, i, i+offset, offset);
    for (int i=offset; i < n-offset; i += 2*offset) // compare even-odd regions
      compareRegions(data, n, i, i+offset, offset);
  }

#ifdef notdef
  for (unsigned k = 2; k <= size; k <<= 1) // k is doubled every iteration
  {
    for (unsigned j = k/2; j > 0; j >>= 1) // j is halved at every iteration, with truncation of fractional parts
    {
      for (unsigned i = 0; i < size; i++)
      {
        unsigned l = (i ^ j);
#ifndef VIP_DO_MODE
        if ((l > i) && ((((i & k) == 0) && (data[i] > data[l])) || (((i & k) != 0) && (data[i] < data[l]))) )
        {
          VIP_ENCINT tmp = data[i];
          data[i] = data[l];
          data[l] = tmp;
          swaps++;
        }
#else /* VIP_DO_MODE */
        VIP_ENCBOOL _pred = ((l > i) && ((((i & k) == 0) && (data[i] > data[l])) || (((i & k) != 0) && (data[i] < data[l]))) );
        VIP_ENCINT tmp = data[i];
        data[i] = VIP_CMOV(_pred, data[l], data[i]);
        data[l] = VIP_CMOV(_pred, tmp, data[l]);
        swaps++;
#endif /* VIP_DO_MODE */
      }
    }
  }
#endif /* notdef */
}

int
main(void)
{
  // initialize the privacy enhanced execution target
  VIP_INIT;

  // initialize the pseudo-RNG
  mysrand(42);
  // mysrand(time(NULL));

  // initialize the array to sort
  for (unsigned i=0; i < DATASET_SIZE; i++)
    data[i] = myrand();
  print_data(data, DATASET_SIZE);

  {
    Stopwatch s("VIP_Bench Runtime");
    randshellsort(data, DATASET_SIZE);
  }
  print_data(data, DATASET_SIZE);

  // check the array
  for (unsigned i=0; i < DATASET_SIZE-1; i++)
  {
    if (VIP_DEC(data[i]) > VIP_DEC(data[i+1]))
    {
      fprintf(stdout, "ERROR: data is not properly sorted.\n");
      return -1;
    }
  }
  fprintf(stderr, "INFO: %lu swaps executed.\n", swaps);
  fprintf(stdout, "INFO: data is properly sorted.\n");
  return 0;
}
