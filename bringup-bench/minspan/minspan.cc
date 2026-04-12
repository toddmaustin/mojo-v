#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"

//
// Compute the minimal spanning tree of a randomly generated graph.
// This program implements Kruscal's algorithm (https://en.wikipedia.org/wiki/Kruskal%27s_algorithm)
// over a graph represented as an adjacency matrix (https://en.wikipedia.org/wiki/Adjacency_matrix).
// This algorithm works on graphs with multiple components that are not connected.
// 

// V represents the number of vertex in the graph G
const int V = 10;

// this are the names(representation) of each vertex
const char *vertName[V] = { "Home    ", "z-mall  ", "st.pet  ", "office  ", "school  ", "motel   ", "restr.  ", "library ", "airport ", "barber  " };

// find the vertex with min distance from the unknown vertexes
int64e_t
minVal(int64e_t dist[V], int64e_t known[V])
{
	int64e_t min = -1;
	int64e_t distVal = INT_MAX;
			
	int64e_t condition = false;	
	for(int i=0;i<V;i++){
		condition = (distVal>dist[i])&&!known[i];
		distVal = cmov(condition,dist[i],distVal);
		min = cmov(condition, (int64e_t)i, min);					
	}
		
	for(int i=0;i<V;i++){
		condition = (min==i);
		known[i] = cmov(condition,int64e_t(true),known[i]);
	}
  return min;
}

// find the shortest path from the source to all other vertexes
void
minSpanTree(int64e_t graph[V][V], int64e_t path[V])
{
	int64e_t dist[V];

  // KNOWN[I] set to true when the algorithm has linked node I into the minimal spanning tree being built
	int64e_t known[V];
	int64e_t min = 0;
	
	int64e_t condition=false;
		
	//sets the source vertex as known and gives it a distance of 0;	
	for(int i=0;i<V;i++){
		condition = (min==i);
		dist[i] = cmov(condition,int64e_t(0),int64e_t(INT_MAX));
		known[i] = cmov(condition,int64e_t(true),int64e_t(false));	
	}
		
	for(int k = 0;k<V;k++){	
		for(int i=0;i<V;i++){
			for(int j=0;j<V;j++){
				
				//This states if the ith element is the min vertex from the unknowns and 
				//if it has a connection with element j(graph[i][j]!=0) and if the distance is smaller than
				//the previous distance then update the path and the distance					 
				condition = (min==i) && !known[j] && (graph[i][j]!=0) && (graph[i][j] < dist[j]);
				dist[j]=cmov(condition,graph[i][j],dist[j]);
				path[j]=cmov(condition,min,path[j]);
			}
		}
		min = minVal(dist, known);
	}
}

//Used to initialize the graph
void
initializeData(int64e_t graph[V][V])
{
	for (int i=0; i < V; i++)
  {
		for (int j=0;j<V;j++)
    {
			if (i>j)
				graph[i][j] = graph[j][i];
			else if (i==j)
				graph[i][j] = 0;
			else
      {
				if (libmin_rand() % 5 == libmin_rand() % 5)
					graph[i][j] = 0;	
				else
					graph[i][j] = libmin_rand() % 10;			
			}
		}
	}
}

void
displayGraph(int64e_t graph[V][V])
{
	int index = 0;
	for (int i=-1;i<V;i++)
  {
		for (int j=-1;j<V;j++)
    {
			if (i==-1)
      {
				if (j==-1)
					libmin_printf("       ");
				else
					libmin_printf("%s", vertName[j]);
			}
      else
      {
				if(j==-1)
        {
					libmin_printf("%s", vertName[index]);
					index++;
				}
        else
					libmin_printf("%8ld", (graph[i][j]).decrypt());
			}
		}
    libmin_printf("\n");
	}
  libmin_printf("\n\n");
}

void
displayGraph1(int64e_t graph[V][V], int64e_t path[V])
{
	int index = 0;
	for (int i=-1;i<V;i++)
  {
		for (int j=-1;j<V;j++)
    {
			if (i==-1)
      {
				if (j==-1)
          libmin_printf("       ");
				else
					libmin_printf("%s", vertName[j]);
			}
      else
      {
				if(j==-1)
        {
					libmin_printf("%s", vertName[index]);
					index++;
				}
        else
          libmin_printf("%8ld/%ld", (graph[i][j]).decrypt(), path[i].decrypt());
			}
		}
    libmin_printf("\n");
	}
  libmin_printf("\n\n");
}

//Displays the path from source to destination
void
displayPath(int64e_t source, int64e_t destination, int64e_t path[V])
{
	int sourceF = source.decrypt();
	int destF = destination.decrypt();
	int count = 0;
	
	int currPath = destination.decrypt();
	
	if(count == 0){
		libmin_printf("\nSource: %s\tDestination: %s\n\n", vertName[sourceF], vertName[currPath]);
		libmin_printf("Path: %s", vertName[sourceF]);
		count++;
	}	
	if(path[currPath].decrypt() != sourceF){
		displayPath(sourceF,path[currPath],path);
	}
	libmin_printf("-> %s", vertName[currPath]);
	if(currPath == destF){
		libmin_printf("\n");
	}
}

// display the minimum spanning tree
void
displayTree(int64e_t graph[V][V], int64e_t path[V])
{
  int cost = 0;
  libmin_printf("minimum spanning tree:\n");
  for (int i=1; i < V; i++)
  {
    libmin_printf("  %8s <-%d-> %8s\n", vertName[i], (graph[i][path[i].decrypt()]).decrypt(), vertName[path[i].decrypt()]);
    cost += (graph[i][path[i].decrypt()]).decrypt();
  }
  libmin_printf("total cost = %d\n", cost);
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

  // initialize input graph
	int64e_t graph[V][V];
	int64e_t path[V];	
	for(int i=0;i<V;i++){
		path[i]=-1;
	}
	initializeData(graph);	

  // show inputs
	displayGraph(graph);

	{
		// Stopwatch start("VIP_BENCH_RUN_TIME");
		minSpanTree(graph,path);
	}

  // produce outputs
	// displayPath(source,destination,path);
	// displayGraph1(graph, path);
	displayTree(graph, path);

  libmin_success();
  return 0;
}
