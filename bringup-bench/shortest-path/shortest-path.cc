#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"

// Number of vertices in the graph
#define V 8
 
/* Define Infinite as a large enough value. This value will be used
  for vertices not connected to each other */
#define INF 99999
#define N 99999

  /* Let us create the following weighted graph
          10
     (0)------->(3)
      |         /|\
    5 |          |
      |          | 1
     \|/         |
     (1)------->(2)
          3           */
int64e_t graph[V][V];
int64_t _graph[V][V] = {
  // Vertex # A  B  C  D  E  F  G  H	   Vertex
            { 0, N, 4, N, N, 7, N, N }, // A
			      { N, 0, N, N, 9, N, N, 3 }, // B
			      { 4, N, 0, 3, N, 2, 9, N }, // C	
			      { N, N, 3, 0, 3, N, 7, N }, // D
			      { N, 9, N, 3, 0, N, 2, 7 }, // E
			      { 7, N, 2, N, N, 0, 8, N }, // F
			      { N, N, 9, 7, 2, 8, 0, 3 }, // G
			      { N, 3, N, N, 7, N, 3, 0 } };//H

// A function to print the solution matrix
void printSolution(int64e_t dist[][V]);
 
// distance array
int64e_t dist[V][V];

// Solves the all-pairs shortest path problem using Floyd Warshall algorithm
void
floydWarshall (int64e_t graph[][V])
{
  /* dist[][] will be the output matrix that will finally have the shortest 
    distances between every pair of vertices */
  int i, j, k;
 
  /* Initialize the solution matrix same as input graph matrix. Or 
     we can say the initial values of shortest distances are based
     on shortest paths considering no intermediate vertex. */
  for (i = 0; i < V; i++)
    for (j = 0; j < V; j++)
      dist[i][j] = graph[i][j];
 
  /* Add all vertices one by one to the set of intermediate vertices.
    ---> Before start of a iteration, we have shortest distances between all
    pairs of vertices such that the shortest distances consider only the
    vertices in set {0, 1, 2, .. k-1} as intermediate vertices.
    ----> After the end of a iteration, vertex no. k is added to the set of
    intermediate vertices and the set becomes {0, 1, 2, .. k} */
  for (k = 0; k < V; k++)
  {
    // Pick all vertices as source one by one
    for (i = 0; i < V; i++)
    {
      // Pick all vertices as destination for the
      // above picked source
      for (j = 0; j < V; j++)
      {
        // If vertex k is on the shortest path from
        // i to j, then update the value of dist[i][j]
        int64e_t _pred = (dist[i][k] + dist[k][j] < dist[i][j]);
        dist[i][j] = cmov(_pred, dist[i][k] + dist[k][j], dist[i][j]);
      }
    }
  }
}
 
/* A utility function to print solution */
void
printSolution(int64e_t dist[][V])
{
    libmin_printf ("Following matrix shows the shortest distances"
                   " between every pair of vertices \n");
    for (int i = 0; i < V; i++)
    {
        for (int j = 0; j < V; j++)
        {
            if ((dist[i][j]).decrypt() == INF)
                libmin_printf("%7s", "INF");
            else
                libmin_printf ("%7d", (dist[i][j]).decrypt());
        }
        libmin_printf("\n");
    }
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

  // initialize graph data
  for (unsigned i=0; i<V; i++)
    for (unsigned j=0; j<V; j++)
      graph[i][j] = _graph[i][j];

  {
    // Stopwatch s("VIP_Bench Runtime");

    // Print the solution
    floydWarshall(graph);
  }
 
  // Print the shortest distance matrix
  printSolution(dist);

  libmin_success();
  return 0;
}
