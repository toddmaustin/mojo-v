#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "utils.h"

// include build configuration defines
#include "../config.h"

// FIXME: hack
#ifdef VIP_NA_MODE
#define VIP_CMOV(A,B,C)   ((A) ? (B) : (C))
#endif

// size of esch individual experiment
#define EXPSIZE 1000000

void
slice_n_dice(FILE *outfile, uint64_t samples)
{
  VIP_ENCINT x, y, z;
  VIP_ENCFLOAT f, g, h;
  VIP_ENCBOOL a, b, c, d;
  uint64_t count = 0, tripCount = 0;

  // continue this test sequence until 
  while (count < samples)
  {

    fprintf(stdout, "INFO: Running the too-many-0s test...\n");
    for (unsigned i=0; i < EXPSIZE; i++)
    {
      x = 1;
      VIP_EMITCT(outfile, x, count, samples, done);
    }

    fprintf(stdout, "INFO: Running the integer fixed-stride test...\n");
    x = 0;
    for (unsigned i=0; i < EXPSIZE; i++)
    {
      x = x + 1;
      VIP_EMITCT(outfile, x, count, samples, done);
    }

    fprintf(stdout, "INFO: Running the negative integer fixed-stride test...\n");
    x = 0;
    for (unsigned i=0; i < EXPSIZE; i++)
    {
      x = x - 1;
      VIP_EMITCT(outfile, x, count, samples, done);
    }

    fprintf(stdout, "INFO: Running the generalized integer computation test...\n");
    x = 0;
    for (unsigned i=0; i < EXPSIZE; i++)
    {
      y = x*x + 4*x - 5;
      VIP_EMITCT(outfile, x, count, samples, done);
      VIP_EMITCT(outfile, y, count, samples, done);
      x++;
    }

    fprintf(stdout, "INFO: Running the integer CMOV test...\n");
    a = false;
    x = 0; y = 1;
    for (unsigned i=0; i < EXPSIZE; i++)
    {
      z = VIP_CMOV(a, x, y);
      a = !a; x = x + 1; y = y - 1;
      VIP_EMITCT(outfile, a, count, samples, done);
      VIP_EMITCT(outfile, x, count, samples, done);
      VIP_EMITCT(outfile, y, count, samples, done);
      VIP_EMITCT(outfile, z, count, samples, done);
    }

    fprintf(stdout, "INFO: Running the too-many-0.0s test...\n");
    for (unsigned i=0; i < EXPSIZE; i++)
    {
      f = 1;
      VIP_EMITCT(outfile, f, count, samples, done);
    }

    fprintf(stdout, "INFO: Running the FP fixed-stride test...\n");
    f = 0.0f;
    for (unsigned i=0; i < EXPSIZE; i++)
    {
      f = f + 1.0f;
      VIP_EMITCT(outfile, f, count, samples, done);
    }

    fprintf(stdout, "INFO: Running the negative FP fixed-stride test...\n");
    f = 0.0f;
    for (unsigned i=0; i < EXPSIZE; i++)
    {
      f = f - 1.0f;
      VIP_EMITCT(outfile, f, count, samples, done);
    }

    fprintf(stdout, "INFO: Running the generalized FP computation test...\n");
    f = 0.0f;
    for (unsigned i=0; i < EXPSIZE; i++)
    {
      g = f*f + 4.0f*f - 5;
      VIP_EMITCT(outfile, f, count, samples, done);
      VIP_EMITCT(outfile, g, count, samples, done);
      f = f + 1.0f;
    }

    fprintf(stdout, "INFO: Running the FP CMOV test...\n");
    a = false;
    f = 0.0f; g = 1.0f;
    for (unsigned i=0; i < EXPSIZE; i++)
    {
      h = VIP_CMOV(a, f, g);
      a = !a; f = f + 1.0f; g = g - 1.0f;
      VIP_EMITCT(outfile, a, count, samples, done);
      VIP_EMITCT(outfile, f, count, samples, done);
      VIP_EMITCT(outfile, g, count, samples, done);
      VIP_EMITCT(outfile, h, count, samples, done);
    }

    fprintf(stdout, "INFO: Running the too-many-falses test...\n");
    for (unsigned i=0; i < EXPSIZE; i++)
    {
      a = false;
      VIP_EMITCT(outfile, a, count, samples, done);
    }

    fprintf(stdout, "INFO: Running the boolean ping-pong test...\n");
    a = false;
    for (unsigned i=0; i < EXPSIZE; i++)
    {
      a = !a;
      VIP_EMITCT(outfile, a, count, samples, done);
    }

    fprintf(stdout, "INFO: Running the generalized boolean computation test...\n");
    for (unsigned i=0; i < EXPSIZE; i++)
    {
      a = true;
      b = (a && a) || a;
      VIP_EMITCT(outfile, a, count, samples, done);
      VIP_EMITCT(outfile, b, count, samples, done);
      a = false;
      b = (a || !a) && a;
      VIP_EMITCT(outfile, a, count, samples, done);
      VIP_EMITCT(outfile, b, count, samples, done);
    }

    fprintf(stdout, "INFO: Running the boolean CMOV test...\n");
    a = false;
    b = false; c = true;
    for (unsigned i=0; i < EXPSIZE; i++)
    {
      d = VIP_CMOV(a, b, c);
      a = !a; b = !b; c = !c;
      VIP_EMITCT(outfile, a, count, samples, done);
      VIP_EMITCT(outfile, b, count, samples, done);
      VIP_EMITCT(outfile, c, count, samples, done);
      VIP_EMITCT(outfile, d, count, samples, done);
    }

    tripCount++;
  }

done:
  assert (count == samples);

  // indicate the trip count, should be greater than 1!
  fprintf(stderr, "INFO: tripCount = %lu\n", tripCount);
  if (tripCount == 0)
    fprintf(stderr, "WARNING: trip count should be at least 1!\n");
  return;
}

int
main(int argc, char* argv[])
{
  // initialize the privacy enhanced execution target
  VIP_INIT;

  // initialize the pseudo-RNG
  mysrand(42);
  // mysrand(time(NULL));

  if (argc != 3)
  {
    fprintf(stderr, "USAGE: %s <out-file-name> <sample-size>\n", argv[0]);
    exit(1);
  }

  char *outname = argv[1];
  uint64_t samples = strtoull(argv[2], NULL, 10);;

  fprintf(stdout, "INFO: writing `%lu' samples to file `%s'\n", samples, outname);

  // open the sample file
  FILE *outfile = fopen(outname, "w");
  if (!outfile)
  {
    fprintf(stderr, "ERROR: cannot open file `%s'\n", outname);
    exit(1);
  }

  // write the DIEHARDER RNG sample file header
  fprintf(outfile, "# ciphertext samples from VIP-Bench slice-n-dice\n");
  fprintf(outfile, "type: d\n");
  fprintf(outfile, "count: %lu\n", samples);
  fprintf(outfile, "numbit: 32\n");

  {
    Stopwatch s("VIP_Bench Runtime");
    slice_n_dice(outfile, samples);
  }
  fprintf(stdout, "INFO: slice-n-dice ciphertext sampling is complete.\n");

  // close the ciphertext sample file
  fclose(outfile);
  return 0;
}
