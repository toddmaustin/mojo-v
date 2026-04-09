#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utils.h"

// include build configuration defines
#include "../config.h"

// supported sizes: 256, 512, 1024 (default), 2048
#define DATASET_SIZE 1024
VIP_ENCINT data[DATASET_SIZE];

// total swaps executed so far
unsigned long swaps = 0;


void
print_data(VIP_ENCINT *data, unsigned size)
{
  fprintf(stdout, "DATA DUMP:\n");
  for (unsigned i=0; i < size; i++)
    fprintf(stdout, "  data[%u] = %d\n", i, VIP_DEC(data[i]));
}

// given an array arr of length n, this code sorts it in place
// all indices run from 0 to n-1
void
bitonicsort(VIP_ENCINT *data, unsigned size)
{
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
        VIP_ENCBOOL _pred = ((VIP_ENCBOOL)(l > i) && (((VIP_ENCBOOL)((i & k) == 0) && (data[i] > data[l])) || ((VIP_ENCBOOL)((i & k) != 0) && (data[i] < data[l]))) );
        VIP_ENCINT tmp = data[i];
        data[i] = VIP_CMOV(_pred, data[l], data[i]);
        data[l] = VIP_CMOV(_pred, tmp, data[l]);
        swaps++;
#endif /* VIP_DO_MODE */
      }
    }
  }
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
    bitonicsort(data, DATASET_SIZE);
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
