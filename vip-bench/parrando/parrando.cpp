/* parrando.c: simulation of J. Parrando's probability paradox */


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

/* compile: cc -o parrondo parrondo.c 

      Use -D_NO_RANDOM if your library doesn't have random/srandom. Most do,
       	but the only truly portable RNG is rand/srand. Unfortunately it has
        very poor performance, so you should use random if possible.

      Use -D_MAX_RAND=  to set the size of the maximum value returned by
         random(). The portable RNG rand() always returns a maximum of 
         RAND_MAX (defined in stdlib.h), but some implementations of random
         do not use this value. Read the man page for random to be sure. A
	 common value is 2^31-1 = 2147483647. In so, and this is not the
         value of RAND_MAX on your system, you would compile with
         -D_MAX_RAND=214748367.

      Use -D_SHORT_STRINGS if your compiler does not support multiline
          string constants.
*/


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../config.h"

#define VERSION "1.1"
#define USAGE "parrondo [ -s number -t number -m number -1 -2 -h -v]"
#ifndef _SHORT_STRINGS
#define HELP "parrondo [ -s number -t number -m number -1 -2 ]\n\n\
Print information on simulations of Parrondo's paradoxical game.\n\n\
-s: Use next argument as RNG seed. (otherwise use system time as seed.)\n\
-t: Use next argument as number of trials. Default 10000.\n\
-m: Use number as max fortune (win), -number as min fortune(loss). Default 50. \n\
-v: Print version number and exit. \n\
-h: Print this helpful information. \n\
-1: Simulate simple game alone.\n\
-2: Simulate complex game alone.\n\n"
#else
#define HELP USAGE
#endif

#define RANDOM myrand
#define SRANDOM mysrand

#define MAX_ITERATIONS 1000L   //Iterations per trial
#define TRIALS 100             //Trials per run
#define RUNS   10              //Runs
#define INITIAL_SEED 1
#ifndef _MAX_RAND
#define _MAX_RAND RAND_MAX
#endif


/* Default values */
int MAX_FORTUNE 	  = 50;


/* See above for meaning of these */
double S_WIN_PROB 	  = 0.495;
double BAD_WIN_PROB   = 0.095;
double GOOD_WIN_PROB  = 0.745;




double getrand()
{
	double U;   /* U(0,1) random variable */
	U = ((double)RANDOM())/((double)_MAX_RAND);

	return U;
}

VIP_ENCINT cointoss(VIP_ENCDOUBLE p, double U)
{
#ifndef VIP_DO_MODE
  VIP_ENCINT ret = p<=U ? -1 : 1;
#else
	VIP_ENCINT ret = VIP_CMOV(p<=U, (VIP_ENCINT)-1, (VIP_ENCINT)1);
#endif
	return ret;
} 

/* One play of the simple game: +1 if win, -1 if loss. */
VIP_ENCINT play_s(double U)
{
	return cointoss(S_WIN_PROB, U);
}

/* One play of the complicated game: +1 if win, -1 if loss. */
VIP_ENCINT play_c(VIP_ENCINT fortune, double U)
{
#ifndef VIP_DO_MODE
	if( fortune % 3 )
		return cointoss(GOOD_WIN_PROB, U);
	return cointoss(BAD_WIN_PROB, U);
#else
  VIP_ENCINT ret = VIP_CMOV( (fortune % 3) != 0, cointoss(GOOD_WIN_PROB, U), cointoss(BAD_WIN_PROB, U));
  return ret; 
#endif
}
	


int
main(void)
{
  VIP_INIT;

  VIP_ENCINT n=0;
	VIP_ENCDOUBLE n_bar, n_tot=0.0;
	VIP_ENCINT win_count = 0;
	VIP_ENCINT loss_count = 0;
	VIP_ENCINT site_visits[3];                   // Counts visits to numbers mod 3 
  for(int i=0;i<3;i++) site_visits[i] = 0;     // Initialize counters 
	VIP_ENCINT fortune = 0;
	
  /* Governs a coin toss below which selects between games. 
     Setting this to 1.0 chooses complex game only. Setting 
     to 0.0 chooses simple game only. 
	*/  
	VIP_ENCDOUBLE game_select = 0.5;                               


  /*** Prepare random number generator ***/
	SRANDOM((int)INITIAL_SEED);
  int SEED_STREAM[TRIALS*RUNS];
  for(int i=0; i<TRIALS*RUNS; i++)
  {
    SEED_STREAM[i] = RANDOM();
  }


{ Stopwatch s("VIP_Bench Runtime");
	for(int i=0; i<RUNS; i++)
  {
    /*** Prepare run variables ***/
		win_count = 0;
		loss_count = 0;
		fortune = 0;
		n=0;
		n_bar = n_tot=0.0;
		printf("Simulating %d trials ...\n", TRIALS);

		for(int j=0; j<TRIALS; j++)
    { 
			/*** Prepare trial variables ***/
      // Re-seed PRNG with seed stream
			SRANDOM(SEED_STREAM[(i*TRIALS)+j]);
      // Reset iteration count variable, n
      n=0;
      // Reset fortune
    	fortune = 0;
  		// For each trial, loop until fortune goes out of range (e.g., beyond MAX_FORTUNE)
			VIP_ENCBOOL done = false; 

      /*** Begin trials ***/
			for(int k=0; k<MAX_ITERATIONS; k++)
      {
        // Coin toss to select between conplex or simple game
        VIP_ENCBOOL cond = (cointoss(game_select, getrand())==1); 
				double U = getrand(); 
#ifndef VIP_DO_MODE
        if (cond) 
          fortune += play_c(fortune,U);
        else
          fortune += play_s(U);
#else
				VIP_ENCINT if_result = play_c(fortune, U); 
				VIP_ENCINT else_result = play_s(U);
				fortune += VIP_CMOV(!done, VIP_CMOV(cond, if_result, else_result), (VIP_ENCINT) 0);
#endif
        // Check if fortune has gone out of range (e.g., beyond MAX_FORTUNE)
#ifndef VIP_DO_MODE
        done = (fortune >= MAX_FORTUNE)||(fortune <= -MAX_FORTUNE);
        if(VIP_DEC(done))
          break;
#else
				done = VIP_CMOV(!done && ((fortune >= MAX_FORTUNE)||(fortune <= -MAX_FORTUNE)), (VIP_ENCBOOL)true, done); 
#endif
        // Document site visits 
#ifndef VIP_DO_MODE
        VIP_ENCINT m = (fortune > 0) ? fortune : -fortune;
        site_visits[m%3]++;	
#else
				VIP_ENCINT m = VIP_CMOV(fortune > 0, fortune, -fortune);
        VIP_ENCINT m_index = m%3;
        site_visits[0] = site_visits[0] + VIP_CMOV(!done && m_index==0, (VIP_ENCINT)1, (VIP_ENCINT)0);
        site_visits[1] = site_visits[1] + VIP_CMOV(!done && m_index==1, (VIP_ENCINT)1, (VIP_ENCINT)0);
        site_visits[2] = site_visits[2] + VIP_CMOV(!done && m_index==2, (VIP_ENCINT)1, (VIP_ENCINT)0);
#endif
        // Increment iteration count variable, n
#ifndef VIP_DO_MODE
        n++;
#else 
        n += VIP_CMOV(!done, (VIP_ENCINT)1, (VIP_ENCINT)0); 
#endif
			} // Iteration loop (k)



      /*** Aggregate stats for Trial-j ***/
      // Increment total iteration count with results from this trial
      n_tot = n_tot + n;
      // Increment win count/loss count with results from this trial
#ifndef VIP_DO_MODE
			if(fortune == MAX_FORTUNE)
				win_count++;
			else if(fortune == -MAX_FORTUNE)
				loss_count++;
#else
      VIP_ENCBOOL cond1 = (fortune == MAX_FORTUNE); 
			VIP_ENCBOOL cond2 = (fortune == -MAX_FORTUNE);
			win_count =  VIP_CMOV(cond1, win_count+1, win_count);
			loss_count = VIP_CMOV(!cond1 && cond2, loss_count+1, loss_count);
#endif
		} // Trial loop (j)
	
    

		/*** Print Results of this run ***/
    n_bar = n_tot/((double)TRIALS);
		printf("%d wins, %d losses, %d draws\n",
            VIP_DEC(win_count),
				    VIP_DEC(loss_count), 
            TRIALS-VIP_DEC(win_count+loss_count)
    );
		printf("Average trial length = %g\n",VIP_DEC(n_bar));
		int sv_0 = VIP_DEC(site_visits[0]);
		int sv_1 = VIP_DEC(site_visits[1]);
		int sv_2 = VIP_DEC(site_visits[2]);
		printf("Site occupancy: 0 mod 3: %g%%, 1 mod 3: %g%%, 2 mod 3: %g%%\n",
			      100.0*((double)sv_0)/VIP_DEC(n_tot),			//*** n_tot is a loop count, so not d-o
			      100.0*((double)sv_1)/VIP_DEC(n_tot),			//*** site_visits is. Here we are printing result
			      100.0*((double)sv_2)/VIP_DEC(n_tot)
		);

	} // Run loop (i)
}


	return 0;

}

