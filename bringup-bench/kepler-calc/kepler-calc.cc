#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"
#include "mojov-math.h"

#define VERSION "1.11"

#ifndef PI
#define PI 3.14159265359
#endif
#define LAPLACE_LIMIT .6627434193
#define USAGE "kepler [-h -v -a <.nnnn...> -m <k>] M e"

#ifdef _SHORT_STRINGS
#define HELP USAGE
#else
#define HELP USAGE"\n\
-h: print this helpful message\n\
-v: print version number and exit\n\
-a: obtain solution to accuracy of  < .nnnn (default .0000001)\n\
-m: use selected calculation method k, where\n\
  k = 1: Simple iteration.\n\
  k = 2: Newton's method.\n\
  k = 3: Binary search.\n\
  k = 4: Series in powers of e. (e<.6627434193.)\n\
  k = 5: Fourier Bessel series.\n\
M = mean anomaly (radians)\n\
e = orbit eccentricty."
#endif

/* E = eccentric anomaly,
   e = eccentricity,
   M = mean anomaly
   (using radian measure).
*/

double bin_fact(int n, int j);  /* Used with e-series method. See below. */
fp64e_t J(int, fp64e_t);           /* Calculates Bessel function. */
#define MAXITER     16
static double derror = 0.000001;

/* All the algorithms for solving kepler's equation are implemented in
   the following subroutines. A subroutine is called iteratively from
   main, until the previous value of E differs from the current one by
   less than derror.

   To add a new method, add its implementation as a subroutine 
   with signature

  double foo(double E, double e, double M, int reset);

   It should return a better approximation to the true E
   given the current E and the values of e and M. When passed a 
   nonzero value for the reset parameter it should reinitialize any
   static information it retains and return.
   Then add the address of
   the new subroutine to the methods array defined below.
*/

/* CURRENTLY IMPLEMENTED METHODS: */

/* Used to solve kepler's equation by simple iteration */

fp64e_t strict_iteration(fp64e_t E, fp64e_t e, fp64e_t M, int reset)
{

  /* reset is not used in this routine. It may generate a compiler
           warning */
  return M + e*mojov_sin(E);
}
/* The following routine is used to solve kepler's equation using
   Newton's method. It is very fast and reliable for small values of
   e, but can be wildly erratic for e close to 1. See, e.g, the discussion
   in Jean Meeus, Astronomical Algorithms, Willmann-Bell, 1991, 181-193.
*/

fp64e_t newton(fp64e_t E, fp64e_t e, fp64e_t M, int reset)
{
  /* reset is not used in this routine. It may generate a compiler
           warning */
  return E + (M + e*mojov_sin(E) - E)/((fp64e_t)1 - e*mojov_cos(E));
}

/* The following routine implements the binary search algorithm due
   to Roger Sinnott, Sky and Telescope, Vol 70, page 159 (August 1985.)
   It is not the fastest algorithm, but it is completely reliable. 
*/

fp64e_t binary(fp64e_t E, fp64e_t e, fp64e_t M, int reset)
{
  static double scale = .7853981633975;   /* PI/4 */
  fp64e_t R;
  fp64e_t X;

  // reset is not private
  if(reset) {
    scale = PI/4.0;
    return 0.0;
  }

  R = E - e*mojov_sin(E);
  X = cmov(M > R, (E + scale), (E - scale));
  scale = scale/2.0;
  return X;
}

/* The following infinite series expansion for E in powers of e is known:

                 __
          \       n
  E = M +      A e
    /__   n
    n=1

where              __ 
             n-1  \          k            (n-1)
A =      (1/2 n!)        (-1) C(n,k)(n-2k)   sin((n-2k)M),
 n                /__
                 0<= k <= [n/2]

which converges for e < LAPLACE_LIMIT (defined above). This is based upon
the Laplace inversion formula -- see the discussion of Burmann's theorem
and following material in Whittaker and Watson.

The bin_fact helper routine calculates C(n,k)(n-2k)^(n-1)/n!2^(n-1) */

fp64e_t e_series(fp64e_t E, fp64e_t e, fp64e_t M, int reset)
{
  static int n;
  int k;
  fp64e_t n_2k,a_n=0.0,s_k;

  // reset is not private
  if(reset){
    n = 0;
    return 0.0;
  }

  // n is not private
  if (n==0)
  {
    n++;
    return M;
  }


  for(k=0;2*k<=n;k++){
    n_2k = (double)n - 2.0 * ((double)k);
    // k is not private
    s_k = k%2 ? -1.0 : 1.0;   /*   (-1)^k */
    a_n = a_n + (s_k*bin_fact(n,k)*mojov_sin(n_2k*M));
  }
  n++;
  return E + mypow(e,n-1)*a_n;
}

/* The eccentric anomaly is an odd periodic function in the Mean Anomoly
   and so can be developed in a Fourier sine series. This turns out to
   be 
                 __
          \  
  E = M +      (2/n)J (ne)sin(nM)
    /__        n 
    n=1

  where J_n is the Bessel function of the first kind. See, e.g, A Mathematical
  Introdution to Celestial Mechanics, Harry Pollard, Prentice Hall, 1966,
  pp 22-23. The following routine is used to sum this series.
*/


fp64e_t j_series(fp64e_t E, fp64e_t e, fp64e_t M, int reset)
{
  static int n;
  fp64e_t dn, term;

  // reset is not private
  if(reset){
    n = 0;
    return 0.0;
  }

  if (n==0)
  {
    n++;
    return M;
  }

  dn = (double)n;
  term = ((fp64e_t)2.0/(double)n)*J(n,dn*e)*mojov_sin(dn*M);
  n++;
  return E + term;
}

typedef fp64e_t (*method_fn)(fp64e_t, fp64e_t, fp64e_t, int);
static method_fn methods[] = {
          strict_iteration,
          newton,
          binary,
          e_series,
          j_series,
        };

#define NMETHODS (sizeof methods /sizeof(void *))

/* Symbolic constants for method indices. */
#define IITERATION 0
#define INEWTON 1
#define IBINARY 2
#define IESERIES 3
#define IJSERIES 4

int newmain(int argc, const char **argv);

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

  // test parameters
  libmin_printf("Solve Kepler's Eq via simple iteration for test parameters...\n");
  {
    int argc=5;
    const char *argv[] = { "kepler", "-m", "1", "0.34", "0.25" };

    newmain(argc, argv);
  }

  // SpaceX Tesla orbit
  libmin_printf("Solve Kepler's Eq via Newton's method for SpaceX Tesla...\n");
  {
    int argc=5;
    const char *argv[] = { "kepler", "-m", "2", "6.037831992006549", "0.25600674983752" };

    newmain(argc, argv);
  }

  // Haley's comet
  libmin_printf("Solve Kepler's Eq via binary search for Haley's comet...\n");
  {
    int argc=5;
    const char *argv[] = { "kepler", "-m", "3", "0.66985737", "0.96714" };

    newmain(argc, argv);
  }

  // Earth's orbit
  libmin_printf("Solve Kepler's Eq via power series for Earth's orbit...\n");
  {
    int argc=5;
    const char *argv[] = { "kepler", "-m", "4", "6.259047404", "0.0167086" };

    newmain(argc, argv);
  }

  // Pluto's orbit
  libmin_printf("Solve Kepler's Eq via Fourier Bessel series for Pluto's orbit...\n");
  {
    int argc=5;
    const char *argv[] = { "kepler", "-m", "5", "0.25359634", "0.2488" };

    newmain(argc, argv);
  }

  libmin_success();
  return 0;
}

int
newmain(int argc, const char **argv)
{
  int i=1;
  int m=1;
  fp64e_t sign = 1.0;
  fp64e_t M = 0.0;
  double _e = -0.1;
  fp64e_t e = -0.1;
  fp64e_t E_old=PI/2;
  fp64e_t E;
  fp64e_t (*method)(fp64e_t,fp64e_t, fp64e_t,int);


  /* Process command line options */

  while(argv[i][0] == '-'){
      if(strcmp(argv[i],"-h")==0){
      printf("%s\n", HELP);
      exit(0);
      }
      if(strcmp(argv[i],"-v")==0){
      printf("%s\n",VERSION);
      exit(0);
      }
      if(strcmp(argv[i],"-a")==0){
      derror = atof(argv[i+1]);
      if(derror <= DBL_EPSILON)
              fprintf(stderr,"Warning: requested precision may exceed implementation limit.\n");
      i += 2;
      continue;
      }
      if(strcmp(argv[i],"-m")==0){
      m = atoi(argv[i+1]);
      if((m<=0) || (m>(int)NMETHODS)){
        fprintf(stderr,"Bad method number %d\n",m);
        return 1;
      }
      i += 2;
      continue;
      }
      fprintf(stderr, "kepler: Unknown option %s\n", argv[i]);
      fprintf(stderr, "%s\n",USAGE);
      return 1;
  }
  if(i + 2 > argc){
    fprintf(stderr, "%s\n",USAGE);
    return 1;
  }
  {
    // Stopwatch s("VIP_Bench Runtime");
  M = atof(argv[i++]);
  e = _e = atof(argv[i]);
  method = (uint64e_t(*)(uint64e_t,uint64e_t,uint64e_t,int))methods[m-1];

  if((m==4)&&(_e > LAPLACE_LIMIT)){
    fprintf(stderr,"e cannot exceed %f for this method.\n",
        LAPLACE_LIMIT);
    return 1;
  }

  if((_e<0)||(_e>=1.0)){
    fprintf(stderr,"Eccentricity %f out of range.\n",_e);
    return 1;
  }

  /* Normalize M to lie between 0 and PI */
  sign = cmov(M > 0, (fp64e_t)1.0, (fp64e_t)-1.0);
  M = myfabs(M)/((fp64e_t)2*PI);
  M = (M - myfloor(M))*2*PI*sign;
  sign = 1.0;
  uint64e_t _pred = M > PI;
  M = cmov(_pred, (fp64e_t)2*PI - M, M);
  sign = cmov(_pred, (fp64e_t)-1.0, sign);

  /* Do selected calculation, and quit when accuracy is bettered. */
  for (unsigned iter=0; iter < MAXITER; iter++)
    {
      E = method(E_old, e, M, 0);
      E_old = E;
    }
    printf("E = %f\n",(sign*E).decrypt());
}
  return 0;
}

/* The bin_fact routine calculates C(n,k)(n-2k)^(n-1)/n!2^(n-1). This
   routine assumes 2k <  n, and tries to keep the intermediate products
   small to prevent overflow.  */

double
bin_fact(int n, int k)
{
    int j;
    double cum_prod = 1.0;
    double num_fact,den_fact,dj,dk,x;

    x = ((double) n)/2.0 - (double)k;

    for(j=n-k;j>1;j--){
      dj = (double)j;
      dk = (double) n -(double)k - dj + 1.0;
      den_fact = n - k - j + 1 <= k ? dk*dj : dj;
      num_fact = n - k - j + 1 <= k ? x*x : x;
      cum_prod = cum_prod * (num_fact/den_fact);
    }
    return cum_prod;
}

/* The following routine calculates the Bessel function of the first kind 
   for an integer index. We just sum the series representation given by


                      __                     2j
                \        j        (x/2)
J (x) = 1/n! (x/2)^n       (-1)   __________________
 n                /__          j!(n+1)...(n+j)
          j=0

   
See Special functions and their applications, N.N. Lebedev, Dover, 1972,
pp 95-142 for an introduction to Bessel functions and related cylinder
functions.

*/
#define MAXJITER    12

fp64e_t J(int n, fp64e_t x)
{
  fp64e_t dsum=0.0,dterm,s_j,d_n,d_j,cfact=1.0;
  int j,nn;

  nn = n >= 0 ? n : -n;  /* Absolute value of n. Use the relation
                                  J  (x) = (-1)^n J  (x) for negative n 
            -n              n    */

  d_n = (double) nn;

  /* Calculate the common factor (x/2)^n/n! so it only has to be
           done once. */

  for(j=1;j<=nn;j++){
    d_j = (double)j;
    cfact = cfact * (x/((fp64e_t)2.0*d_j));
  }

  /* j = 0 term: */
  dsum = dterm = cfact;

  j = 1;

  do {
    d_j = (double)j;
    s_j = j%2 ? -1.0: 1.0;
    dterm = dterm * (x*x/(d_j*4.0*(d_n + d_j)));
    dsum = dsum + (s_j*dterm);
    j++;
  } while( j < MAXJITER );
  // fprintf(stderr, "j == %d\n", j);

  s_j = nn%2 ? -1.0 : 1.0;
  return  n >= 0 ? dsum : s_j*dsum;
}

/* The bin_fact routine calculates C(n,k)(n-2k)^(n-1)/n!2^(n-1). This
 *    routine assumes 2k <  n, and tries to keep the intermediate products
 *       small to prevent overflow.  */

double
bin_fact(int n, int k)
{
    int j;
    double cum_prod = 1.0;
    double num_fact,den_fact,dj,dk,x;

    x = ((double) n)/2.0 - (double)k;

    for(j=n-k;j>1;j--){
      dj = (double)j;
      dk = (double) n -(double)k - dj + 1.0;
      den_fact = n - k - j + 1 <= k ? dk*dj : dj;
      num_fact = n - k - j + 1 <= k ? x*x : x;
      cum_prod = cum_prod * (num_fact/den_fact);
    }
    return cum_prod;
}

/* The following routine calculates the Bessel function of the first kind 
 *    for an integer index. We just sum the series representation given by
 *
 *
 *                          __                     2j
 *                                          \        j        (x/2)
 *                                          J (x) = 1/n! (x/2)^n       (-1)
 *                                          __________________
 *                                           n                /__          j!
 *                                           (n+1)...(n+j)
 *                                                     j=0
 *
 *                                                        
 *                                                        See Special
 *                                                        functions and their
 *                                                        applications, N.N.
 *                                                        Lebedev, Dover, 1972,
 *                                                        pp 95-142 for an
 *                                                        introduction to
 *                                                        Bessel functions and
 *                                                        related cylinder
 *                                                        functions.
 *
 *                                                        */
#define MAXJITER    12

uint64e_t J(int n, uint64e_t x)
{
  uint64e_t dsum=0.0,dterm,s_j,d_n,d_j,cfact=1.0;
  int j,nn;

  nn = n >= 0 ? n : -n;  /* Absolute value of n. Use the relation
                                  J  (x) = (-1)^n J  (x) for negative n 
            -n              n    */

  d_n = (double) nn;

  /* Calculate the common factor (x/2)^n/n! so it only has to be
 *            done once. */

  for(j=1;j<=nn;j++){
    d_j = (double)j;
    cfact = cfact * (x/((uint64e_t)2.0*d_j));
  }

  /* j = 0 term: */
  dsum = dterm = cfact;

  j = 1;

  do {
    d_j = (double)j;
    s_j = j%2 ? -1.0: 1.0;
    dterm = dterm * (x*x/(d_j*4.0*(d_n + d_j)));
    dsum = dsum + (s_j*dterm);
    j++;
  } while( j < MAXJITER );

  s_j = nn%2 ? -1.0 : 1.0;
  return  n >= 0 ? dsum : s_j*dsum;
}

