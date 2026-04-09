#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <vector>
#include "utils.h"

/* Period parameters */  
#define N 624
#define M 397
#define MATRIX_A 0x9908b0df   /* constant vector a */
#define UPPER_MASK 0x80000000 /* most significant w-r bits */
#define LOWER_MASK 0x7fffffff /* least significant r bits */

/* Tempering parameters */   
#define TEMPERING_MASK_B 0x9d2c5680
#define TEMPERING_MASK_C 0xefc60000
#define TEMPERING_SHIFT_U(y)  (y >> 11)
#define TEMPERING_SHIFT_S(y)  (y << 7)
#define TEMPERING_SHIFT_T(y)  (y << 15)
#define TEMPERING_SHIFT_L(y)  (y >> 18)

static bool mt_initialized = false;
static unsigned int mt[N+1]; /* the array for the state vector  */
static int mti=N+1; /* mti==N+1 means mt[N] is not initialized */

double Stopwatch::timeTaken=0;
bool Stopwatch::precision=true;
int Stopwatch::numIter=1;
int Stopwatch::nSlots=1;
bool Stopwatch::tableFormat=true;


/* Initializing the array with a seed */
void
mysrand(unsigned int seed)
{
  int i;

  mt_initialized = true;
  for (i=0;i<N;i++)
  {
    mt[i] = seed & 0xffff0000;
    seed = 69069 * seed + 1;
    mt[i] |= (seed & 0xffff0000) >> 16;
    seed = 69069 * seed + 1;
  }
  mti = N;
}

unsigned int 
myrand(void)
{
  if (!mt_initialized)
  {
    fprintf(stderr, "ERROR: rng is not initialized, call mysrand()!\n");
    exit(1);
  }

  unsigned int y;
  static unsigned int mag01[2]={0x0, MATRIX_A};
  /* mag01[x] = x * MATRIX_A  for x=0,1 */

  if (mti >= N) /* generate N words at one time */
  {
    int kk;

    if (mti == N+1)   /* if sgenrand() has not been called, */
      mysrand(4357); /* a default initial seed is used   */

    for (kk=0;kk<N-M;kk++)
    {
      y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
      mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1];
    }
    for (;kk<N-1;kk++)
    {
      y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
      mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1];
    }
    y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
    mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1];

    mti = 0;
  }
  
  y = mt[mti++];
  y ^= TEMPERING_SHIFT_U(y);
  y ^= TEMPERING_SHIFT_S(y) & TEMPERING_MASK_B;
  y ^= TEMPERING_SHIFT_T(y) & TEMPERING_MASK_C;
  y ^= TEMPERING_SHIFT_L(y);

  return y; 
}

long
perf_event_open(struct perf_event_attr *hw_event, pid_t pid, int cpu, int group_fd, unsigned long flags)
{
  int ret;
  ret = syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
  return ret;
} 
 void record_mem(){
                //if (mem_count > 100) return;
                //mem_count++;
                char system_cmd[1024];
                int sprintf_ret = snprintf(
                        system_cmd,
                        sizeof(system_cmd),
                        "grep -E 'VmPeak|VmRSS|VmSize' /proc/%ju/status >> mem.out",
                        (uintmax_t)getpid()
               );
               assert(sprintf_ret >= 0);
               assert((size_t)sprintf_ret < sizeof(system_cmd));
               //puts(system_cmd);
               if (system(system_cmd))
               {
                 fprintf(stderr, "Cannot start memory monitor.\n");
                 exit(1);
               }
               std::ostringstream ss;
               ss<< std::ifstream("mem.out").rdbuf();
               std::string command_output=ss.str();
               // size_t loc=0, loc2;
               // loc=command_output.find("\n");
               // loc2=command_output.substr(loc+1).find("\n");
               //std::cerr << "[VIP] " << command_output.substr(loc+1,loc2)<<"\n";
               
}

void record_mem(std::string fileName ){
                //if (mem_count > 100) return;
                //mem_count++;
               
               std::string command("ps -o pid,vsz,rss | awk '{if (NR == 1 || $1 == \"%ju\") print}' > ");
                command+=fileName;

                printf("%s\n",command.c_str());
                char system_cmd[1024];
                int sprintf_ret = snprintf(
                        system_cmd,
                        sizeof(system_cmd),
                        command.c_str(),
                        (uintmax_t)getpid()
               );
               assert(sprintf_ret >= 0);
               assert((size_t)sprintf_ret < sizeof(system_cmd));
               //puts(system_cmd);
               if (system(system_cmd))
               {
                 fprintf(stderr, "Cannot start memory monitor.\n");
                 exit(1);
               }
               //puts("");
}
/*int record_mem(){
	// if (mem_count > 100) return;
	// mem_count++;

	char system_cmd[1024];
  int sprintf_ret = snprintf(
          system_cmd,
          sizeof(system_cmd),
          "ps -o pid,vsz,rss | awk '{if (NR == 1 || $1 == \"%ju\") print}' >> mem.out",  
          (uintmax_t)getpid()
  );
  assert(sprintf_ret >= 0);
  assert((size_t)sprintf_ret < sizeof(system_cmd));
  return system(system_cmd);
}*/
