/*
This program counts the number of triangles found in a graph. 
Multiplied the adjacency matrix to find the cube of the matrix then find 
the trace(main diagonal summation) and divide by 6.
*/
#include <iostream>

#include "../config.h"

#define SIZE 13
using namespace std;

VIP_ENCINT squared[SIZE][SIZE];
VIP_ENCINT cubed[SIZE][SIZE];

void
multiply(VIP_ENCINT matrix1[][SIZE], VIP_ENCINT matrix2[][SIZE], VIP_ENCINT product[][SIZE])
{
	for(unsigned i = 0; i<SIZE; i++){
		for(unsigned j = 0; j<SIZE; j++){
			product[i][j] = 0;
			for(unsigned k = 0; k<SIZE; k++){
				product[i][j] = (VIP_ENCINT)(product[i][j] + matrix1[i][k] * matrix2[k][j]);
			}
		}
	}
}

VIP_ENCINT
trace(VIP_ENCINT cubed[][SIZE])
{
	VIP_ENCINT sum = 0;
	for(unsigned i = 0; i<SIZE; i++){
		sum = (VIP_ENCINT)(sum + cubed[i][i]);
	}
	return sum;
}

VIP_ENCINT
count(VIP_ENCINT trace)
{
	return (trace / 6);
}

int main()
{
	VIP_INIT; 
		
	VIP_ENCINT graph[SIZE][SIZE] = {
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
	{0,0,0,0,0,0,0,0,0,0,1,0,0}};
	//{{0,1,1,0},{1,0,1,1},{1,1,0,1},{0,1,1,0}};
	VIP_ENCINT triangleCount;
	{
		Stopwatch s("VIP_Bench Runtime");
		multiply(graph,graph,squared);
		multiply(squared,graph,cubed);
		VIP_ENCINT diagonal = trace(cubed);
		triangleCount = count(diagonal);
	}
	cout<<"The number of triangles is: "<<VIP_DEC(triangleCount)<<endl;
  return 0;
}

