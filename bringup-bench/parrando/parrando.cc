#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"

/* Parrondo's game is based upon two simple games of chance.

        The gambler's fortune starts at 0.

	The simple game: Toss a biased coin and win +1 with probability
		S_WIN_PROB (defined below). Otherwise win -1; 

	The complex game: If the player's fortune is divisible by 3, toss
		the "bad coin" having win probability BAD_WIN_PROB.
		If the player's fortune is not divisible by 3 toss the
                "good coin" having win probability GOOD_WIN_PROB.

	A game ends when the accumlated fortune exceeds MAX_FORTUNE ( a "win" )
	or dips below -MAX_FORTUNE ( a "loss .)  

	The numbers are chosen so that each game is quite unfavorable.
        Remarkably, when the games are alternated at random, the resulting
        game is quite favorable.

	This program simulates many trials of a Parrondo game and reports
        statistics on their outcomes. (It can also simulate the simple and
        complex games individually.) The number of trials, fortune limits,
        and a seed for the random number can be supplied on the command line.
        The -h option prints detailed help. 

	For more information on Parrondo games and related phenomena see
        J. Parrondo's website, http://seneca.fis.ucm.es/parr/,  or the
        announcement in Nature magazine, 23/30, December 1999.
*/


#define MAX_ITERATIONS 1000L   //Iterations per trial
#define TRIALS 100             //Trials per run
#define RUNS   10              //Runs
#define INITIAL_SEED 1

/* Default values */
int MAX_FORTUNE 	  = 50;

/* See above for meaning of these */
double S_WIN_PROB 	  = 0.495;
double BAD_WIN_PROB   = 0.095;
double GOOD_WIN_PROB  = 0.745;

double getrand()
{
	double U;   /* U(0,1) random variable */
	U = ((double)libmin_rand())/((double)RAND_MAX);

	return U;
}

int64e_t cointoss(fp64e_t p, double U)
{
	int64e_t ret = cmov(p<=U, (int64e_t)-1, (int64e_t)1);
	return ret;
} 

/* One play of the simple game: +1 if win, -1 if loss. */
int64e_t play_s(double U)
{
	return cointoss(S_WIN_PROB, U);
}

/* One play of the complicated game: +1 if win, -1 if loss. */
int64e_t play_c(int64e_t fortune, double U)
{
  int64e_t ret = cmov( (fortune % 3) != 0, cointoss(GOOD_WIN_PROB, U), cointoss(BAD_WIN_PROB, U));
  return ret; 
}

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

  int64e_t n=0;
	fp64e_t n_bar, n_tot=0.0;
	int64e_t win_count = 0;
	int64e_t loss_count = 0;
	int64e_t site_visits[3];                   // Counts visits to numbers mod 3 
  for(int i=0;i<3;i++) site_visits[i] = 0;     // Initialize counters 
	int64e_t fortune = 0;
	
  /* Governs a coin toss below which selects between games. 
     Setting this to 1.0 chooses complex game only. Setting 
     to 0.0 chooses simple game only. 
	*/  
	fp64e_t game_select = 0.5;                               


  /*** Prepare random number generator ***/
	libmin_srand((int)INITIAL_SEED);
  int SEED_STREAM[TRIALS*RUNS];
  for(int i=0; i<TRIALS*RUNS; i++)
    SEED_STREAM[i] = libmin_rand();

  {
    // Stopwatch s("cmov Runtime");

	  for(int i=0; i<RUNS; i++)
    {
      /*** Prepare run variables ***/
		  win_count = 0;
		  loss_count = 0;
		  fortune = 0;
		  n=0;
		  n_bar = n_tot=0.0;
		  libmin_printf("Simulating %d trials ...\n", TRIALS);

		  for(int j=0; j<TRIALS; j++)
      { 
			  /*** Prepare trial variables ***/
        // Re-seed PRNG with seed stream
			  libmin_srand(SEED_STREAM[(i*TRIALS)+j]);
        // Reset iteration count variable, n
        n=0;
        // Reset fortune
    	  fortune = 0;
  		  // For each trial, loop until fortune goes out of range (e.g., beyond MAX_FORTUNE)
			  int64e_t done = false; 
  
        /*** Begin trials ***/
			  for(int k=0; k<MAX_ITERATIONS; k++)
        {
          // Coin toss to select between conplex or simple game
          int64e_t cond = (cointoss(game_select, getrand())==1); 
				  double U = getrand(); 
				  int64e_t if_result = play_c(fortune, U); 
				  int64e_t else_result = play_s(U);
				  fortune += cmov(!done, cmov(cond, if_result, else_result), (int64e_t)0);

          // Check if fortune has gone out of range (e.g., beyond MAX_FORTUNE)
				  done = cmov(!done && ((fortune >= MAX_FORTUNE)||(fortune <= -MAX_FORTUNE)), (int64e_t)true, done); 

          // Document site visits 
				  int64e_t m = cmov(fortune > 0, fortune, -fortune);
          int64e_t m_index = m%3;
          site_visits[0] = site_visits[0] + cmov(!done && m_index==0, (int64e_t)1, (int64e_t)0);
          site_visits[1] = site_visits[1] + cmov(!done && m_index==1, (int64e_t)1, (int64e_t)0);
          site_visits[2] = site_visits[2] + cmov(!done && m_index==2, (int64e_t)1, (int64e_t)0);

          // Increment iteration count variable, n
          n += cmov(!done, (int64e_t)1, (int64e_t)0); 
			  } // Iteration loop (k)

        /*** Aggregate stats for Trial-j ***/

        // Increment total iteration count with results from this trial
        n_tot = n_tot + (fp64e_t)n;

        // Increment win count/loss count with results from this trial
        int64e_t cond1 = (fortune == MAX_FORTUNE); 
			  int64e_t cond2 = (fortune == -MAX_FORTUNE);
			  win_count =  cmov(cond1, win_count+1, win_count);
			  loss_count = cmov(!cond1 && cond2, loss_count+1, loss_count);
		  } // Trial loop (j)
  
		    /*** Print Results of this run ***/
      n_bar = n_tot/((double)TRIALS);
		  libmin_printf("%ld wins, %ld losses, %ld draws\n",
              win_count.decrypt(), loss_count.decrypt(), TRIALS-win_count.decrypt()+loss_count.decrypt());
		  libmin_printf("Average trial length = %lf\n", n_bar.decrypt());
		  int sv_0 = site_visits[0].decrypt();
		  int sv_1 = site_visits[1].decrypt();
		  int sv_2 = site_visits[2].decrypt();
		  libmin_printf("Site occupancy: 0 mod 3: %lf%%, 1 mod 3: %lf%%, 2 mod 3: %lf%%\n",
			        100.0*((double)sv_0)/n_tot.decrypt(),			//*** n_tot is a loop count, so not d-o
			        100.0*((double)sv_1)/n_tot.decrypt(),			//*** site_visits is. Here we are printing result
			        100.0*((double)sv_2)/n_tot.decrypt());
  
	  } // Run loop (i)
  }

  libmin_success();
  return 0;
}
