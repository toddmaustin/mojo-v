/* A C-program for MT19937: Integer version (1999/10/28)          */
/* genrand() generates one pseudorandom unsigned integer (32bit) */
/* which is uniformly distributed among 0 to 2^32-1  for each     */
/* call. sgenrand(seed) sets initial values to the working area   */
/* of 624 words. Before genrand(), sgenrand(seed) must be         */
/* called once. (seed is any 32-bit integer.)                     */
/*   Coded by Takuji Nishimura, considering the suggestions by    */
/* Topher Cooper and Marc Rieffel in July-Aug. 1997.              */

/* This library is free software; you can redistribute it and/or   */
/* modify it under the terms of the GNU Library General Public     */
/* License as published by the Free Software Foundation; either    */
/* version 2 of the License, or (at your option) any later         */
/* version.                                                        */
/* This library is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of  */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.            */
/* See the GNU Library General Public License for more details.    */
/* You should have received a copy of the GNU Library General      */
/* Public License along with this library; if not, write to the    */
/* Free Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA   */ 
/* 02111-1307  USA                                                 */

/* Copyright (C) 1997, 1999 Makoto Matsumoto and Takuji Nishimura. */
/* Any feedback is very welcome. For any question, comments,       */
/* see http://www.math.keio.ac.jp/matumoto/emt.html or email       */
/* matumoto@math.keio.ac.jp                                        */

/* REFERENCE                                                       */
/* M. Matsumoto and T. Nishimura,                                  */
/* "Mersenne Twister: A 623-Dimensionally Equidistributed Uniform  */
/* Pseudo-Random Number Generator",                                */
/* ACM Transactions on Modeling and Computer Simulation,           */
/* Vol. 8, No. 1, January 1998, pp 3--30.                          */


#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <iomanip>
using namespace std;

#include "../config.h"

/* Period parameters */  
#define N 624
#define M 397

VIP_ENCUINT MATRIX_A;     /* constant vector a */
VIP_ENCUINT UPPER_MASK;   /* most significant w-r bits */
VIP_ENCUINT LOWER_MASK;   /* least significant r bits */

/* Tempering parameters */   
VIP_ENCUINT TEMPERING_MASK_B;
VIP_ENCUINT TEMPERING_MASK_C;
VIP_ENCUINT TEMPERING_SHIFT_U;
VIP_ENCUINT TEMPERING_SHIFT_S;
VIP_ENCUINT TEMPERING_SHIFT_T;
VIP_ENCUINT TEMPERING_SHIFT_L;

static VIP_ENCUINT mt[N];   /* the array for the state vector  */

/*USED FOR ARRAY ACCESS, NOT SECRET*/
static int         mti=N+1; /* mti==N+1 means mt[N] is not initialized */


/* Initializing the array with a seed */
void
sgenrand(VIP_ENCUINT seed)
{
  int i;

  for (i=0;i<N;i++)
    {
      mt[i] = seed & 0xffff0000;
      seed = (VIP_ENCUINT)69069 * seed + 1;
      mt[i] = mt[i] | ((seed & 0xffff0000) >> 16);
      seed = (VIP_ENCUINT)69069 * seed + 1;
    }
  mti = N;
}


VIP_ENCUINT 
genrand(void)
{
  VIP_ENCUINT y;

  if (mti >= N) { /* generate N words at one time */
    int kk;

    if (mti == N+1)   /* if sgenrand() has not been called, */
      abort(); // sgenrand((VIP_ENCUINT)4357); /* a default initial seed is used   */

    for (kk=0;kk<N-M;kk++){
	      y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
	      mt[kk] = mt[kk+M] ^ (y >> 1) ^ ((y & 0x1)*MATRIX_A);
    }
    for (;kk<N-1;kk++){
	      y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
	      mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ ((y & 0x1)*MATRIX_A); 
    }

    y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
    mt[N-1] = mt[M-1] ^ (y >> 1) ^ ((y & 0x1)*MATRIX_A);

    mti = 0;
  }
  
  y = mt[mti++];
  y = y ^ (y >> TEMPERING_SHIFT_U);
  y = y ^ ((y << TEMPERING_SHIFT_S) & TEMPERING_MASK_B);
  y = y ^ ((y << TEMPERING_SHIFT_T) & TEMPERING_MASK_C);
  y = y ^ (y >> TEMPERING_SHIFT_L);

  return y; 
}


int
main(void) 
{ 
  VIP_INIT; 
  MATRIX_A   = 0x9908b0df;   /* constant vector a */
  UPPER_MASK = 0x80000000;   /* most significant w-r bits */
  LOWER_MASK = 0x7fffffff;   /* least significant r bits */
  
  /* Tempering parameters */   
  TEMPERING_MASK_B  = 0x9d2c5680;
  TEMPERING_MASK_C  = 0xefc60000;
  TEMPERING_SHIFT_U = 11;
  TEMPERING_SHIFT_S = 7;
  TEMPERING_SHIFT_T = 15;
  TEMPERING_SHIFT_L = 18;

  int steps = 1000;
  int i, j;
  
  {
    Stopwatch s("VIP_Bench Runtime");
    VIP_ENCUINT seedval = 42;
    sgenrand(seedval);

    VIP_ENCUINT randval;
    for (i=0,j=0; i<steps; i++)
    {
      randval = genrand();
      fprintf(stdout, "%10u, ", VIP_DEC(randval));
      if (++j%8==0)
        cout << endl;;
    }
  }
  return 0;
} 

