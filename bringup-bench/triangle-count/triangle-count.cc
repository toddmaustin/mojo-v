#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"

#define SIZE 13
int64e_t graph[SIZE][SIZE];
int64_t _graph[SIZE][SIZE] =
{
  {0,1,0,0,1,0,0,0,0,0,0,0,0},
	{1,0,1,1,0,0,0,0,0,0,0,0,0},
	{0,1,0,1,0,0,1,0,0,0,0,0,0},
	{0,1,1,0,0,0,0,0,0,0,0,0,0},
	{1,0,0,0,0,1,0,1,0,0,0,0,0},
	{0,0,0,0,1,0,1,1,0,0,0,0,0},
	{0,0,1,0,0,1,0,0,0,1,1,0,0},
	{0,0,0,0,1,1,0,0,1,0,0,0,0},
	{0,0,0,0,0,0,0,1,0,0,0,0,0},
	{0,0,0,0,0,0,1,0,0,0,1,1,0},
	{0,0,0,0,0,0,1,0,0,1,0,0,1},
	{0,0,0,0,0,0,0,0,0,1,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,1,0,0}
};
	
int64e_t squared[SIZE][SIZE];
int64e_t cubed[SIZE][SIZE];

void
multiply(int64e_t matrix1[][SIZE], int64e_t matrix2[][SIZE], int64e_t product[][SIZE])
{
	for(unsigned i = 0; i<SIZE; i++){
		for(unsigned j = 0; j<SIZE; j++){
			product[i][j] = 0;
			for(unsigned k = 0; k<SIZE; k++){
				product[i][j] = (int64e_t)(product[i][j] + matrix1[i][k] * matrix2[k][j]);
			}
		}
	}
}

int64e_t
trace(int64e_t cubed[][SIZE])
{
	int64e_t sum = 0;
	for(unsigned i = 0; i<SIZE; i++){
		sum = (int64e_t)(sum + cubed[i][i]);
	}
	return sum;
}

int64e_t
count(int64e_t trace)
{
	return (trace / 6);
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

  // initialize the encrypted inputs
  for (int i=0; i<SIZE; i++)
    for (int j=0; j<SIZE; j++)
      graph[i][j] = _graph[i][j];

	int64e_t triangleCount;
	{
		// Stopwatch s("VIP_Bench Runtime");

		multiply(graph,graph,squared);
		multiply(squared,graph,cubed);
		int64e_t diagonal = trace(cubed);
		triangleCount = count(diagonal);
	}
	libmin_printf("INFO: The number of triangles is: %lu\n", triangleCount.decrypt());

  libmin_success();
  return 0;
}
