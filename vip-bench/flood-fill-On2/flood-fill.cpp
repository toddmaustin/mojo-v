#include <iostream>
#include <iomanip>
#include <cstdint>
#include <vector>

using namespace std;

#include "../config.h"
#include "utils.h"

// sizes support 10 (default), 15, 20, 25, 32, 45
// bitmaps needed 1          , 1,  1,  2,  3,  5
#define SIZE 10
int BITMAPS = 1;

// `M × N` matrix
#define M SIZE
#define N SIZE

typedef vector<VIP_ENCUINT64> VIP_Enc_Group_t;
VIP_Enc_Group_t initGroup(BITMAPS, 0); //{BITMAPS x 0}
struct node
{
	VIP_Enc_Group_t group = initGroup;
};

// matrix showing portion of the screen having different colors
VIP_ENCCHAR mat[M][N] =
    {
#if SIZE == 10
        {'Y', 'Y', 'Y', 'G', 'G', 'G', 'G', 'G', 'G', 'G'},
        {'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'G', 'X', 'X', 'X'},
        {'G', 'X', 'G', 'G', 'G', 'G', 'G', 'X', 'X', 'X'},
        {'W', 'X', 'X', 'W', 'W', 'G', 'G', 'G', 'G', 'X'},
        {'W', 'R', 'R', 'R', 'R', 'R', 'G', 'X', 'X', 'X'},
        {'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X'},
        {'W', 'B', 'W', 'R', 'R', 'R', 'R', 'R', 'R', 'X'},
        {'W', 'B', 'B', 'B', 'B', 'R', 'R', 'X', 'X', 'X'},
        {'W', 'B', 'B', 'X', 'B', 'B', 'B', 'B', 'X', 'X'},
        {'W', 'B', 'B', 'X', 'X', 'X', 'X', 'X', 'X', 'X'}
#elif SIZE == 15
        {'Y', 'Y', 'Y', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'Y', 'Y', 'Y', 'G', 'G'},
        {'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'G', 'X', 'X', 'X', 'Y', 'Y', 'Y', 'Y', 'Y'},
        {'G', 'X', 'G', 'G', 'G', 'G', 'G', 'X', 'X', 'X', 'G', 'X', 'G', 'G', 'G'},
        {'W', 'X', 'X', 'W', 'W', 'G', 'G', 'G', 'G', 'X', 'W', 'X', 'X', 'W', 'W'},
        {'W', 'R', 'R', 'R', 'R', 'R', 'G', 'X', 'X', 'X', 'W', 'R', 'R', 'R', 'R'},
        {'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X', 'W', 'W', 'W', 'R', 'R'},
        {'W', 'B', 'W', 'R', 'R', 'R', 'R', 'R', 'R', 'X', 'W', 'B', 'W', 'R', 'R'},
        {'W', 'B', 'B', 'B', 'B', 'R', 'R', 'X', 'X', 'X', 'W', 'B', 'B', 'B', 'B'},
        {'W', 'B', 'B', 'X', 'B', 'B', 'B', 'B', 'X', 'X', 'W', 'B', 'B', 'X', 'B'},
        {'W', 'B', 'B', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'W', 'B', 'B', 'X', 'X'},
        {'Y', 'Y', 'Y', 'G', 'G', 'G', 'G', 'X', 'G', 'G', 'Y', 'Y', 'Y', 'G', 'G'},
        {'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'G', 'X', 'X', 'X', 'Y', 'Y', 'Y', 'Y', 'Y'},
        {'G', 'X', 'G', 'G', 'G', 'G', 'G', 'X', 'X', 'X', 'G', 'X', 'G', 'G', 'G'},
        {'W', 'X', 'X', 'W', 'W', 'G', 'G', 'G', 'G', 'X', 'W', 'X', 'X', 'W', 'W'},
        {'W', 'R', 'R', 'R', 'R', 'R', 'G', 'X', 'X', 'X', 'W', 'R', 'R', 'R', 'R'}
#elif SIZE == 20
        {'Y', 'Y', 'Y', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'Y', 'Y', 'Y', 'G', 'G', 'G', 'G', 'G', 'G', 'G'},
        {'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'G', 'X', 'X', 'X', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'G', 'X', 'X', 'X'},
        {'G', 'X', 'G', 'G', 'G', 'G', 'G', 'X', 'X', 'X', 'G', 'X', 'G', 'G', 'G', 'G', 'G', 'X', 'X', 'X'},
        {'W', 'X', 'X', 'W', 'W', 'G', 'G', 'G', 'G', 'X', 'W', 'X', 'X', 'W', 'W', 'G', 'G', 'G', 'G', 'X'},
        {'W', 'R', 'R', 'R', 'R', 'R', 'G', 'X', 'X', 'X', 'W', 'R', 'R', 'R', 'R', 'R', 'G', 'X', 'X', 'X'},
        {'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X', 'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X'},
        {'W', 'B', 'W', 'R', 'R', 'R', 'R', 'R', 'R', 'X', 'W', 'B', 'W', 'R', 'R', 'R', 'R', 'R', 'R', 'X'},
        {'W', 'B', 'B', 'B', 'B', 'R', 'R', 'X', 'X', 'X', 'W', 'B', 'B', 'B', 'B', 'R', 'R', 'X', 'X', 'X'},
        {'W', 'B', 'B', 'X', 'B', 'B', 'B', 'B', 'X', 'X', 'W', 'B', 'B', 'X', 'B', 'B', 'B', 'B', 'X', 'X'},
        {'W', 'B', 'B', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'W', 'B', 'B', 'X', 'X', 'X', 'X', 'X', 'X', 'X'},
        {'Y', 'Y', 'Y', 'G', 'G', 'G', 'G', 'X', 'G', 'G', 'Y', 'Y', 'Y', 'X', 'G', 'G', 'G', 'X', 'G', 'G'},
        {'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'G', 'X', 'X', 'X', 'Y', 'Y', 'Y', 'X', 'Y', 'Y', 'G', 'X', 'X', 'X'},
        {'G', 'X', 'G', 'G', 'G', 'G', 'G', 'X', 'X', 'X', 'X', 'X', 'G', 'X', 'G', 'G', 'G', 'X', 'X', 'X'},
        {'W', 'X', 'X', 'W', 'W', 'G', 'G', 'G', 'G', 'X', 'W', 'X', 'X', 'X', 'W', 'G', 'G', 'G', 'G', 'X'},
        {'W', 'R', 'R', 'R', 'R', 'R', 'G', 'X', 'X', 'X', 'W', 'R', 'R', 'R', 'R', 'R', 'G', 'X', 'X', 'X'},
        {'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X', 'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X'},
        {'W', 'B', 'W', 'R', 'R', 'R', 'R', 'R', 'R', 'X', 'W', 'B', 'W', 'R', 'R', 'R', 'R', 'R', 'R', 'X'},
        {'W', 'B', 'B', 'B', 'B', 'R', 'R', 'X', 'X', 'X', 'W', 'B', 'B', 'B', 'B', 'R', 'R', 'X', 'X', 'X'},
        {'W', 'B', 'B', 'X', 'B', 'B', 'B', 'B', 'X', 'X', 'W', 'B', 'B', 'X', 'B', 'B', 'B', 'B', 'X', 'X'},
        {'W', 'B', 'B', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'W', 'B', 'B', 'X', 'X', 'X', 'X', 'X', 'X', 'X'}
#elif SIZE == 25
        {'Y', 'Y', 'Y', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'Y', 'Y', 'Y', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'Y', 'Y', 'Y', 'G'},
        {'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'G', 'X', 'X', 'X', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'G', 'X', 'X', 'X', 'X', 'Y', 'Y', 'Y', 'Y'},
        {'G', 'X', 'G', 'G', 'G', 'G', 'G', 'X', 'X', 'X', 'G', 'X', 'G', 'G', 'G', 'G', 'G', 'X', 'X', 'X', 'X', 'G', 'X', 'G', 'G'},
        {'W', 'X', 'X', 'W', 'W', 'G', 'G', 'G', 'G', 'X', 'W', 'X', 'X', 'W', 'W', 'G', 'G', 'G', 'G', 'X', 'X', 'W', 'X', 'X', 'W'},
        {'W', 'R', 'R', 'R', 'R', 'R', 'G', 'X', 'X', 'X', 'W', 'R', 'R', 'R', 'R', 'R', 'G', 'X', 'X', 'X', 'X', 'W', 'R', 'R', 'R'},
        {'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X', 'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X', 'X', 'W', 'W', 'W', 'R'},
        {'W', 'B', 'W', 'R', 'R', 'R', 'R', 'R', 'R', 'X', 'W', 'B', 'W', 'R', 'R', 'R', 'R', 'R', 'R', 'X', 'X', 'W', 'B', 'W', 'R'},
        {'W', 'B', 'B', 'B', 'B', 'R', 'R', 'X', 'X', 'X', 'W', 'B', 'B', 'B', 'B', 'R', 'R', 'X', 'X', 'X', 'X', 'W', 'B', 'B', 'B'},
        {'W', 'B', 'B', 'X', 'B', 'B', 'B', 'B', 'X', 'X', 'W', 'B', 'B', 'X', 'B', 'B', 'B', 'B', 'X', 'X', 'X', 'W', 'B', 'B', 'X'},
        {'W', 'B', 'B', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'W', 'B', 'B', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'W', 'B', 'B', 'X'},
        {'Y', 'Y', 'Y', 'G', 'G', 'G', 'G', 'X', 'G', 'G', 'Y', 'Y', 'Y', 'X', 'G', 'G', 'G', 'X', 'G', 'G', 'G', 'Y', 'Y', 'Y', 'X'},
        {'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'G', 'X', 'X', 'X', 'Y', 'Y', 'Y', 'X', 'Y', 'Y', 'G', 'X', 'X', 'X', 'X', 'Y', 'Y', 'Y', 'X'},
        {'G', 'X', 'G', 'G', 'G', 'G', 'G', 'X', 'X', 'X', 'X', 'X', 'G', 'X', 'G', 'G', 'G', 'X', 'X', 'X', 'X', 'X', 'X', 'G', 'X'},
        {'W', 'X', 'X', 'W', 'W', 'G', 'G', 'G', 'G', 'X', 'W', 'X', 'X', 'X', 'W', 'G', 'G', 'G', 'G', 'X', 'X', 'W', 'X', 'X', 'X'},
        {'W', 'R', 'R', 'R', 'R', 'R', 'G', 'X', 'X', 'X', 'W', 'R', 'R', 'R', 'R', 'R', 'G', 'X', 'X', 'X', 'X', 'W', 'R', 'R', 'R'},
        {'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X', 'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X', 'X', 'W', 'W', 'W', 'R'},
        {'W', 'B', 'W', 'R', 'R', 'R', 'R', 'R', 'R', 'X', 'W', 'B', 'W', 'R', 'R', 'R', 'R', 'R', 'R', 'X', 'X', 'W', 'B', 'W', 'R'},
        {'W', 'B', 'B', 'B', 'B', 'R', 'R', 'X', 'X', 'X', 'W', 'B', 'B', 'B', 'B', 'R', 'R', 'X', 'X', 'X', 'X', 'W', 'B', 'B', 'B'},
        {'W', 'B', 'B', 'X', 'B', 'B', 'B', 'B', 'X', 'X', 'W', 'B', 'B', 'X', 'B', 'B', 'B', 'B', 'X', 'X', 'X', 'W', 'B', 'B', 'X'},
        {'W', 'B', 'B', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'W', 'B', 'B', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'W', 'B', 'B', 'X'},
        {'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'G', 'X', 'X', 'X', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'G', 'X', 'X', 'X', 'X', 'Y', 'Y', 'Y', 'Y'},
        {'G', 'X', 'G', 'G', 'G', 'G', 'G', 'X', 'X', 'X', 'G', 'X', 'G', 'G', 'G', 'G', 'G', 'X', 'X', 'X', 'X', 'G', 'X', 'G', 'G'},
        {'W', 'X', 'X', 'W', 'W', 'G', 'G', 'G', 'G', 'X', 'W', 'X', 'X', 'W', 'W', 'G', 'G', 'G', 'G', 'X', 'X', 'W', 'X', 'X', 'W'},
        {'W', 'R', 'R', 'R', 'R', 'R', 'G', 'X', 'X', 'X', 'W', 'R', 'R', 'R', 'R', 'R', 'G', 'X', 'X', 'X', 'X', 'W', 'R', 'R', 'R'},
        {'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X', 'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X', 'X', 'W', 'W', 'W', 'R'}
#elif SIZE == 32
        {'Y', 'Y', 'Y', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'Y', 'Y', 'Y', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'Y', 'Y', 'Y', 'G', 'Y', 'Y', 'Y', 'G', 'G', 'G', 'G'},
        {'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'G', 'X', 'X', 'X', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'G', 'X', 'X', 'X', 'X', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'G'},
        {'G', 'X', 'G', 'G', 'G', 'G', 'G', 'X', 'X', 'X', 'G', 'X', 'G', 'G', 'G', 'G', 'G', 'X', 'X', 'X', 'X', 'G', 'X', 'G', 'G', 'G', 'X', 'G', 'G', 'G', 'G', 'G'},
        {'W', 'X', 'X', 'W', 'W', 'G', 'G', 'G', 'G', 'X', 'W', 'X', 'X', 'W', 'W', 'G', 'G', 'G', 'G', 'X', 'X', 'W', 'X', 'X', 'W', 'W', 'X', 'X', 'W', 'W', 'G', 'G'},
        {'W', 'R', 'R', 'R', 'R', 'R', 'G', 'X', 'X', 'X', 'W', 'R', 'R', 'R', 'R', 'R', 'G', 'X', 'X', 'X', 'X', 'W', 'R', 'R', 'R', 'W', 'R', 'R', 'R', 'R', 'R', 'G'},
        {'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X', 'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X', 'X', 'W', 'W', 'W', 'R', 'W', 'W', 'W', 'R', 'R', 'G', 'G'},
        {'W', 'B', 'W', 'R', 'R', 'R', 'R', 'R', 'R', 'X', 'W', 'B', 'W', 'R', 'R', 'R', 'R', 'R', 'R', 'X', 'X', 'W', 'B', 'W', 'R', 'W', 'B', 'W', 'R', 'R', 'R', 'R'},
        {'W', 'B', 'B', 'B', 'B', 'R', 'R', 'X', 'X', 'X', 'W', 'B', 'B', 'B', 'B', 'R', 'R', 'X', 'X', 'X', 'X', 'W', 'B', 'B', 'B', 'W', 'B', 'B', 'B', 'B', 'R', 'R'},
        {'W', 'B', 'B', 'X', 'B', 'B', 'B', 'B', 'X', 'X', 'W', 'B', 'B', 'X', 'B', 'B', 'B', 'B', 'X', 'X', 'X', 'W', 'B', 'B', 'X', 'W', 'B', 'B', 'X', 'B', 'B', 'B'},
        {'W', 'B', 'B', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'W', 'B', 'B', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'W', 'B', 'B', 'X', 'W', 'B', 'B', 'X', 'X', 'X', 'X'},
        {'Y', 'Y', 'Y', 'G', 'G', 'G', 'G', 'X', 'G', 'G', 'Y', 'Y', 'Y', 'X', 'G', 'G', 'G', 'X', 'G', 'G', 'G', 'Y', 'Y', 'Y', 'X', 'Y', 'Y', 'Y', 'X', 'G', 'G', 'G'},
        {'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'G', 'X', 'X', 'X', 'Y', 'Y', 'Y', 'X', 'Y', 'Y', 'G', 'X', 'X', 'X', 'X', 'Y', 'Y', 'Y', 'X', 'Y', 'Y', 'Y', 'X', 'Y', 'Y', 'G'},
        {'G', 'X', 'G', 'G', 'G', 'G', 'G', 'X', 'X', 'X', 'X', 'X', 'G', 'X', 'G', 'G', 'G', 'X', 'X', 'X', 'X', 'X', 'X', 'G', 'X', 'X', 'X', 'G', 'X', 'G', 'G', 'G'},
        {'W', 'X', 'X', 'W', 'W', 'G', 'G', 'G', 'G', 'X', 'W', 'X', 'X', 'X', 'W', 'G', 'G', 'G', 'G', 'X', 'X', 'W', 'X', 'X', 'X', 'W', 'X', 'X', 'X', 'W', 'G', 'G'},
        {'W', 'R', 'R', 'R', 'R', 'R', 'G', 'X', 'X', 'X', 'W', 'R', 'R', 'R', 'R', 'R', 'G', 'X', 'X', 'X', 'X', 'W', 'R', 'R', 'R', 'W', 'R', 'R', 'R', 'R', 'R', 'G'},
        {'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X', 'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X', 'X', 'W', 'W', 'W', 'R', 'W', 'W', 'W', 'R', 'R', 'G', 'G'},
        {'W', 'B', 'W', 'R', 'R', 'R', 'R', 'R', 'R', 'X', 'W', 'B', 'W', 'R', 'R', 'R', 'R', 'R', 'R', 'X', 'X', 'W', 'B', 'W', 'R', 'W', 'B', 'W', 'R', 'R', 'R', 'R'},
        {'W', 'B', 'B', 'B', 'B', 'R', 'R', 'X', 'X', 'X', 'W', 'B', 'B', 'B', 'B', 'R', 'R', 'X', 'X', 'X', 'X', 'W', 'B', 'B', 'B', 'W', 'B', 'B', 'B', 'B', 'R', 'R'},
        {'W', 'B', 'B', 'X', 'B', 'B', 'B', 'B', 'X', 'X', 'W', 'B', 'B', 'X', 'B', 'B', 'B', 'B', 'X', 'X', 'X', 'W', 'B', 'B', 'X', 'W', 'B', 'B', 'X', 'B', 'B', 'B'},
        {'W', 'B', 'B', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'W', 'B', 'B', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'W', 'B', 'B', 'X', 'W', 'B', 'B', 'X', 'X', 'X', 'X'},
        {'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'G', 'X', 'X', 'X', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'G', 'X', 'X', 'X', 'X', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'G'},
        {'G', 'X', 'G', 'G', 'G', 'G', 'G', 'X', 'X', 'X', 'G', 'X', 'G', 'G', 'G', 'G', 'G', 'X', 'X', 'X', 'X', 'G', 'X', 'G', 'G', 'G', 'X', 'G', 'G', 'G', 'G', 'G'},
        {'W', 'X', 'X', 'W', 'W', 'G', 'G', 'G', 'G', 'X', 'W', 'X', 'X', 'W', 'W', 'G', 'G', 'G', 'G', 'X', 'X', 'W', 'X', 'X', 'W', 'W', 'X', 'X', 'W', 'W', 'G', 'G'},
        {'W', 'R', 'R', 'R', 'R', 'R', 'G', 'X', 'X', 'X', 'W', 'R', 'R', 'R', 'R', 'R', 'G', 'X', 'X', 'X', 'X', 'W', 'R', 'R', 'R', 'W', 'R', 'R', 'R', 'R', 'R', 'G'},
        {'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X', 'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X', 'X', 'W', 'W', 'W', 'R', 'W', 'W', 'W', 'R', 'R', 'G', 'G'},
        {'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X', 'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X', 'X', 'W', 'W', 'W', 'R', 'W', 'W', 'W', 'R', 'R', 'G', 'G'},
        {'W', 'B', 'W', 'R', 'R', 'R', 'R', 'R', 'R', 'X', 'W', 'B', 'W', 'R', 'R', 'R', 'R', 'R', 'R', 'X', 'X', 'W', 'B', 'W', 'R', 'W', 'B', 'W', 'R', 'R', 'R', 'R'},
        {'W', 'B', 'B', 'B', 'B', 'R', 'R', 'X', 'X', 'X', 'W', 'B', 'B', 'B', 'B', 'R', 'R', 'X', 'X', 'X', 'X', 'W', 'B', 'B', 'B', 'W', 'B', 'B', 'B', 'B', 'R', 'R'},
        {'W', 'B', 'B', 'X', 'B', 'B', 'B', 'B', 'X', 'X', 'W', 'B', 'B', 'X', 'B', 'B', 'B', 'B', 'X', 'X', 'X', 'W', 'B', 'B', 'X', 'W', 'B', 'B', 'X', 'B', 'B', 'B'},
        {'W', 'B', 'B', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'W', 'B', 'B', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'W', 'B', 'B', 'X', 'W', 'B', 'B', 'X', 'X', 'X', 'X'},
        {'Y', 'Y', 'Y', 'G', 'G', 'G', 'G', 'X', 'G', 'G', 'Y', 'Y', 'Y', 'X', 'G', 'G', 'G', 'X', 'G', 'G', 'G', 'Y', 'Y', 'Y', 'X', 'Y', 'Y', 'Y', 'X', 'G', 'G', 'G'},
        {'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'G', 'X', 'X', 'X', 'Y', 'Y', 'Y', 'X', 'Y', 'Y', 'G', 'X', 'X', 'X', 'X', 'Y', 'Y', 'Y', 'X', 'Y', 'Y', 'Y', 'X', 'Y', 'Y', 'G'}
#elif SIZE == 45
        {'Y', 'Y', 'Y', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'Y', 'Y', 'Y', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'Y', 'Y', 'Y', 'G', 'Y', 'Y', 'Y', 'G', 'G', 'G', 'G', 'G', 'G', 'Y', 'Y', 'Y', 'G', 'Y', 'Y', 'Y', 'G', 'G', 'G', 'G'},
        {'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'G', 'X', 'X', 'X', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'G', 'X', 'X', 'X', 'X', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'G', 'X', 'X', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'G'},
        {'G', 'X', 'G', 'G', 'G', 'G', 'G', 'X', 'X', 'X', 'G', 'X', 'G', 'G', 'G', 'G', 'G', 'X', 'X', 'X', 'X', 'G', 'X', 'G', 'G', 'G', 'X', 'G', 'G', 'G', 'G', 'G', 'X', 'X', 'G', 'X', 'G', 'G', 'G', 'X', 'G', 'G', 'G', 'G', 'G'},
        {'W', 'X', 'X', 'W', 'W', 'G', 'G', 'G', 'G', 'X', 'W', 'X', 'X', 'W', 'W', 'G', 'G', 'G', 'G', 'X', 'X', 'W', 'X', 'X', 'W', 'W', 'X', 'X', 'W', 'W', 'G', 'G', 'X', 'X', 'W', 'X', 'X', 'W', 'W', 'X', 'X', 'W', 'W', 'G', 'G'},
        {'W', 'R', 'R', 'R', 'R', 'R', 'G', 'X', 'X', 'X', 'W', 'R', 'R', 'R', 'R', 'R', 'G', 'X', 'X', 'X', 'X', 'W', 'R', 'R', 'R', 'W', 'R', 'R', 'R', 'R', 'R', 'G', 'X', 'X', 'W', 'R', 'R', 'R', 'W', 'R', 'R', 'R', 'R', 'R', 'G'},
        {'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X', 'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X', 'X', 'W', 'W', 'W', 'R', 'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'W', 'W', 'W', 'R', 'W', 'W', 'W', 'R', 'R', 'G', 'G'},
        {'W', 'B', 'W', 'R', 'R', 'R', 'R', 'R', 'R', 'X', 'W', 'B', 'W', 'R', 'R', 'R', 'R', 'R', 'R', 'X', 'X', 'W', 'B', 'W', 'R', 'W', 'B', 'W', 'R', 'R', 'R', 'R', 'X', 'X', 'W', 'B', 'W', 'R', 'W', 'B', 'W', 'R', 'R', 'R', 'R'},
        {'W', 'B', 'B', 'B', 'B', 'R', 'R', 'X', 'X', 'X', 'W', 'B', 'B', 'B', 'B', 'R', 'R', 'X', 'X', 'X', 'X', 'W', 'B', 'B', 'B', 'W', 'B', 'B', 'B', 'B', 'R', 'R', 'X', 'X', 'W', 'B', 'B', 'B', 'W', 'B', 'B', 'B', 'B', 'R', 'R'},
        {'W', 'B', 'B', 'X', 'B', 'B', 'B', 'B', 'X', 'X', 'W', 'B', 'B', 'X', 'B', 'B', 'B', 'B', 'X', 'X', 'X', 'W', 'B', 'B', 'X', 'W', 'B', 'B', 'X', 'B', 'B', 'B', 'X', 'X', 'W', 'B', 'B', 'X', 'W', 'B', 'B', 'X', 'B', 'B', 'B'},
        {'W', 'B', 'B', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'W', 'B', 'B', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'W', 'B', 'B', 'X', 'W', 'B', 'B', 'X', 'X', 'X', 'X', 'X', 'X', 'W', 'B', 'B', 'X', 'W', 'B', 'B', 'X', 'X', 'X', 'X'},
        {'Y', 'Y', 'Y', 'G', 'G', 'G', 'G', 'X', 'G', 'G', 'Y', 'Y', 'Y', 'X', 'G', 'G', 'G', 'X', 'G', 'G', 'G', 'Y', 'Y', 'Y', 'X', 'Y', 'Y', 'Y', 'X', 'G', 'G', 'G', 'G', 'G', 'Y', 'Y', 'Y', 'X', 'Y', 'Y', 'Y', 'X', 'G', 'G', 'G'},
        {'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'G', 'X', 'X', 'X', 'Y', 'Y', 'Y', 'X', 'Y', 'Y', 'G', 'X', 'X', 'X', 'X', 'Y', 'Y', 'Y', 'X', 'Y', 'Y', 'Y', 'X', 'Y', 'Y', 'G', 'X', 'X', 'Y', 'Y', 'Y', 'X', 'Y', 'Y', 'Y', 'X', 'Y', 'Y', 'G'},
        {'G', 'X', 'G', 'G', 'G', 'G', 'G', 'X', 'X', 'X', 'X', 'X', 'G', 'X', 'G', 'G', 'G', 'X', 'X', 'X', 'X', 'X', 'X', 'G', 'X', 'X', 'X', 'G', 'X', 'G', 'G', 'G', 'X', 'X', 'X', 'X', 'G', 'X', 'X', 'X', 'G', 'X', 'G', 'G', 'G'},
        {'W', 'X', 'X', 'W', 'W', 'G', 'G', 'G', 'G', 'X', 'W', 'X', 'X', 'X', 'W', 'G', 'G', 'G', 'G', 'X', 'X', 'W', 'X', 'X', 'X', 'W', 'X', 'X', 'X', 'W', 'G', 'G', 'X', 'X', 'W', 'X', 'X', 'X', 'W', 'X', 'X', 'X', 'W', 'G', 'G'},
        {'W', 'R', 'R', 'R', 'R', 'R', 'G', 'X', 'X', 'X', 'W', 'R', 'R', 'R', 'R', 'R', 'G', 'X', 'X', 'X', 'X', 'W', 'R', 'R', 'R', 'W', 'R', 'R', 'R', 'R', 'R', 'G', 'X', 'X', 'W', 'R', 'R', 'R', 'W', 'R', 'R', 'R', 'R', 'R', 'G'},
        {'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X', 'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X', 'X', 'W', 'W', 'W', 'R', 'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'W', 'W', 'W', 'R', 'W', 'W', 'W', 'R', 'R', 'G', 'G'},
        {'W', 'B', 'W', 'R', 'R', 'R', 'R', 'R', 'R', 'X', 'W', 'B', 'W', 'R', 'R', 'R', 'R', 'R', 'R', 'X', 'X', 'W', 'B', 'W', 'R', 'W', 'B', 'W', 'R', 'R', 'R', 'R', 'X', 'X', 'W', 'B', 'W', 'R', 'W', 'B', 'W', 'R', 'R', 'R', 'R'},
        {'W', 'B', 'B', 'B', 'B', 'R', 'R', 'X', 'X', 'X', 'W', 'B', 'B', 'B', 'B', 'R', 'R', 'X', 'X', 'X', 'X', 'W', 'B', 'B', 'B', 'W', 'B', 'B', 'B', 'B', 'R', 'R', 'X', 'X', 'W', 'B', 'B', 'B', 'W', 'B', 'B', 'B', 'B', 'R', 'R'},
        {'W', 'B', 'B', 'X', 'B', 'B', 'B', 'B', 'X', 'X', 'W', 'B', 'B', 'X', 'B', 'B', 'B', 'B', 'X', 'X', 'X', 'W', 'B', 'B', 'X', 'W', 'B', 'B', 'X', 'B', 'B', 'B', 'X', 'X', 'W', 'B', 'B', 'X', 'W', 'B', 'B', 'X', 'B', 'B', 'B'},
        {'W', 'B', 'B', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'W', 'B', 'B', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'W', 'B', 'B', 'X', 'W', 'B', 'B', 'X', 'X', 'X', 'X', 'X', 'X', 'W', 'B', 'B', 'X', 'W', 'B', 'B', 'X', 'X', 'X', 'X'},
        {'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'G', 'X', 'X', 'X', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'G', 'X', 'X', 'X', 'X', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'G', 'X', 'X', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'G'},
        {'G', 'X', 'G', 'G', 'G', 'G', 'G', 'X', 'X', 'X', 'G', 'X', 'G', 'G', 'G', 'G', 'G', 'X', 'X', 'X', 'X', 'G', 'X', 'G', 'G', 'G', 'X', 'G', 'G', 'G', 'G', 'G', 'X', 'X', 'G', 'X', 'G', 'G', 'G', 'X', 'G', 'G', 'G', 'G', 'G'},
        {'W', 'X', 'X', 'W', 'W', 'G', 'G', 'G', 'G', 'X', 'W', 'X', 'X', 'W', 'W', 'G', 'G', 'G', 'G', 'X', 'X', 'W', 'X', 'X', 'W', 'W', 'X', 'X', 'W', 'W', 'G', 'G', 'X', 'X', 'W', 'X', 'X', 'W', 'W', 'X', 'X', 'W', 'W', 'G', 'G'},
        {'W', 'R', 'R', 'R', 'R', 'R', 'G', 'X', 'X', 'X', 'W', 'R', 'R', 'R', 'R', 'R', 'G', 'X', 'X', 'X', 'X', 'W', 'R', 'R', 'R', 'W', 'R', 'R', 'R', 'R', 'R', 'G', 'X', 'X', 'W', 'R', 'R', 'R', 'W', 'R', 'R', 'R', 'R', 'R', 'G'},
        {'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X', 'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X', 'X', 'W', 'W', 'W', 'R', 'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'W', 'W', 'W', 'R', 'W', 'W', 'W', 'R', 'R', 'G', 'G'},
        {'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X', 'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X', 'X', 'W', 'W', 'W', 'R', 'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'W', 'W', 'W', 'R', 'W', 'W', 'W', 'R', 'R', 'G', 'G'},
        {'W', 'B', 'W', 'R', 'R', 'R', 'R', 'R', 'R', 'X', 'W', 'B', 'W', 'R', 'R', 'R', 'R', 'R', 'R', 'X', 'X', 'W', 'B', 'W', 'R', 'W', 'B', 'W', 'R', 'R', 'R', 'R', 'X', 'X', 'W', 'B', 'W', 'R', 'W', 'B', 'W', 'R', 'R', 'R', 'R'},
        {'W', 'B', 'B', 'B', 'B', 'R', 'R', 'X', 'X', 'X', 'W', 'B', 'B', 'B', 'B', 'R', 'R', 'X', 'X', 'X', 'X', 'W', 'B', 'B', 'B', 'W', 'B', 'B', 'B', 'B', 'R', 'R', 'X', 'X', 'W', 'B', 'B', 'B', 'W', 'B', 'B', 'B', 'B', 'R', 'R'},
        {'W', 'B', 'B', 'X', 'B', 'B', 'B', 'B', 'X', 'X', 'W', 'B', 'B', 'X', 'B', 'B', 'B', 'B', 'X', 'X', 'X', 'W', 'B', 'B', 'X', 'W', 'B', 'B', 'X', 'B', 'B', 'B', 'X', 'X', 'W', 'B', 'B', 'X', 'W', 'B', 'B', 'X', 'B', 'B', 'B'},
        {'W', 'B', 'B', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'W', 'B', 'B', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'W', 'B', 'B', 'X', 'W', 'B', 'B', 'X', 'X', 'X', 'X', 'X', 'X', 'W', 'B', 'B', 'X', 'W', 'B', 'B', 'X', 'X', 'X', 'X'},
        {'Y', 'Y', 'Y', 'G', 'G', 'G', 'G', 'X', 'G', 'G', 'Y', 'Y', 'Y', 'X', 'G', 'G', 'G', 'X', 'G', 'G', 'G', 'Y', 'Y', 'Y', 'X', 'Y', 'Y', 'Y', 'X', 'G', 'G', 'G', 'G', 'G', 'Y', 'Y', 'Y', 'X', 'Y', 'Y', 'Y', 'X', 'G', 'G', 'G'},
        {'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'G', 'X', 'X', 'X', 'Y', 'Y', 'Y', 'X', 'Y', 'Y', 'G', 'X', 'X', 'X', 'X', 'Y', 'Y', 'Y', 'X', 'Y', 'Y', 'Y', 'X', 'Y', 'Y', 'G', 'X', 'X', 'Y', 'Y', 'Y', 'X', 'Y', 'Y', 'Y', 'X', 'Y', 'Y', 'G'},
        {'W', 'B', 'B', 'X', 'B', 'B', 'B', 'B', 'X', 'X', 'W', 'B', 'B', 'X', 'B', 'B', 'B', 'B', 'X', 'X', 'X', 'W', 'B', 'B', 'X', 'W', 'B', 'B', 'X', 'B', 'B', 'B', 'X', 'X', 'W', 'B', 'B', 'X', 'W', 'B', 'B', 'X', 'B', 'B', 'B'},
        {'W', 'B', 'B', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'W', 'B', 'B', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'W', 'B', 'B', 'X', 'W', 'B', 'B', 'X', 'X', 'X', 'X', 'X', 'X', 'W', 'B', 'B', 'X', 'W', 'B', 'B', 'X', 'X', 'X', 'X'},
        {'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'G', 'X', 'X', 'X', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'G', 'X', 'X', 'X', 'X', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'G', 'X', 'X', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'G'},
        {'G', 'X', 'G', 'G', 'G', 'G', 'G', 'X', 'X', 'X', 'G', 'X', 'G', 'G', 'G', 'G', 'G', 'X', 'X', 'X', 'X', 'G', 'X', 'G', 'G', 'G', 'X', 'G', 'G', 'G', 'G', 'G', 'X', 'X', 'G', 'X', 'G', 'G', 'G', 'X', 'G', 'G', 'G', 'G', 'G'},
        {'W', 'X', 'X', 'W', 'W', 'G', 'G', 'G', 'G', 'X', 'W', 'X', 'X', 'W', 'W', 'G', 'G', 'G', 'G', 'X', 'X', 'W', 'X', 'X', 'W', 'W', 'X', 'X', 'W', 'W', 'G', 'G', 'X', 'X', 'W', 'X', 'X', 'W', 'W', 'X', 'X', 'W', 'W', 'G', 'G'},
        {'W', 'R', 'R', 'R', 'R', 'R', 'G', 'X', 'X', 'X', 'W', 'R', 'R', 'R', 'R', 'R', 'G', 'X', 'X', 'X', 'X', 'W', 'R', 'R', 'R', 'W', 'R', 'R', 'R', 'R', 'R', 'G', 'X', 'X', 'W', 'R', 'R', 'R', 'W', 'R', 'R', 'R', 'R', 'R', 'G'},
        {'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X', 'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X', 'X', 'W', 'W', 'W', 'R', 'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'W', 'W', 'W', 'R', 'W', 'W', 'W', 'R', 'R', 'G', 'G'},
        {'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X', 'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X', 'X', 'W', 'W', 'W', 'R', 'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'W', 'W', 'W', 'R', 'W', 'W', 'W', 'R', 'R', 'G', 'G'},
        {'W', 'B', 'W', 'R', 'R', 'R', 'R', 'R', 'R', 'X', 'W', 'B', 'W', 'R', 'R', 'R', 'R', 'R', 'R', 'X', 'X', 'W', 'B', 'W', 'R', 'W', 'B', 'W', 'R', 'R', 'R', 'R', 'X', 'X', 'W', 'B', 'W', 'R', 'W', 'B', 'W', 'R', 'R', 'R', 'R'},
        {'W', 'B', 'B', 'B', 'B', 'R', 'R', 'X', 'X', 'X', 'W', 'B', 'B', 'B', 'B', 'R', 'R', 'X', 'X', 'X', 'X', 'W', 'B', 'B', 'B', 'W', 'B', 'B', 'B', 'B', 'R', 'R', 'X', 'X', 'W', 'B', 'B', 'B', 'W', 'B', 'B', 'B', 'B', 'R', 'R'},
        {'W', 'B', 'B', 'X', 'B', 'B', 'B', 'B', 'X', 'X', 'W', 'B', 'B', 'X', 'B', 'B', 'B', 'B', 'X', 'X', 'X', 'W', 'B', 'B', 'X', 'W', 'B', 'B', 'X', 'B', 'B', 'B', 'X', 'X', 'W', 'B', 'B', 'X', 'W', 'B', 'B', 'X', 'B', 'B', 'B'},
        {'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X', 'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X', 'X', 'W', 'W', 'W', 'R', 'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'W', 'W', 'W', 'R', 'W', 'W', 'W', 'R', 'R', 'G', 'G'},
        {'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X', 'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X', 'X', 'W', 'W', 'W', 'R', 'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'W', 'W', 'W', 'R', 'W', 'W', 'W', 'R', 'R', 'G', 'G'}

#endif
};


inline VIP_ENCBOOL haveIntersection(VIP_Enc_Group_t bin1, VIP_Enc_Group_t bin2)
{
	VIP_ENCBOOL intersect = false;
	for (int i = 0; i < BITMAPS; i++)
	{
		intersect = intersect || ((bin1[i] & bin2[i]) != 0);
	}

	return intersect;
}
inline VIP_ENCUINT64 combine(VIP_ENCUINT64 bin1, VIP_ENCUINT64 bin2)
{
	return bin1 | bin2;
}






void floodfill(VIP_ENCCHAR mat[M][N], VIP_ENCINT x, VIP_ENCINT y, VIP_ENCCHAR replacement)
{
	int row[] = {-1, -1, -1, 0, 0, 1, 1, 1, 0};
	int col[] = {-1, 0, 1, -1, 1, -1, 0, 1, 0};
	node struct_mat[M][N];
	VIP_ENCUINT64 currId = 1;
	VIP_ENCINT bitMapIdx = 0;

#define SAFELOC(X, Y) ((X) >= 0 && (X) < M && (Y) >= 0 && (Y) < N)

	VIP_ENCBOOL matchFound = false;

	// Forward pass
	for (int i = 0; i < M; i++)
	{
		for (int j = 0; j < N; j++)
		{

			node *cell = &(struct_mat[i][j]);
			VIP_Enc_Group_t commonGroup = cell->group;

			// Forward read pass
			for (int k = 0; k < 4; k++)
			{

				if (SAFELOC(i + row[k], j + col[k]))
				{
					node *adjCell = &(struct_mat[i + row[k]][j + col[k]]);

					VIP_ENCBOOL match = (mat[i][j] == mat[i + row[k]][j + col[k]]);

					matchFound = match || matchFound;

					for (int b = 0; b < BITMAPS; b++)
					{
#ifdef VIP_DO_MODE
						commonGroup[b] = VIP_CMOV(match, combine(commonGroup[b], (adjCell->group)[b]), commonGroup[b]);
#else /* VIP_NA_MODE */
            if (match)
              commonGroup[b] = combine(commonGroup[b], (adjCell->group)[b]);
#endif
					}
				}
			}

			cell->group = commonGroup;

			// Forward write pass
			for (int k = 0; k < 4; k++)
			{
				if (SAFELOC(i + row[k], j + col[k]))
				{
					node *adjCell = &(struct_mat[i + row[k]][j + col[k]]);

					VIP_ENCBOOL match = (mat[i][j] == mat[i + row[k]][j + col[k]]);

					for (int b = 0; b < BITMAPS; b++)
					{
#ifdef VIP_DO_MODE
						(adjCell->group)[b] = VIP_CMOV(match, (commonGroup)[b], (adjCell->group)[b]);
#else /* VIP_NA_MODE */
            if (match)
						  (adjCell->group)[b] = (commonGroup)[b];
#endif
					}
				}
			}

			VIP_ENCBOOL update_current_id = !matchFound && (currId == 0) && (bitMapIdx + 1 < BITMAPS);

#ifdef VIP_DO_MODE
			currId = VIP_CMOV(update_current_id, (VIP_ENCUINT64)1, currId);
#else /* VIP_NA_MODE */
			if (update_current_id)
        currId = (uint64_t)1;
#endif
			// currId = currId + (((currId + 1) << ((((-currId) | currId) >> (64 - 1)) & 1)) & 1);

#ifdef VIP_DO_MODE
      bitMapIdx += VIP_ENCINT(VIP_CMOV(update_current_id, (VIP_ENCINT)1, (VIP_ENCINT)0));
#else /* VIP_NA_MODE */
      bitMapIdx += update_current_id ? 1 : 0;
#endif

			for (int b = 0; b < BITMAPS; b++)
			{
#ifdef VIP_DO_MODE
				cell->group[b] = VIP_CMOV(b == bitMapIdx && !matchFound, currId, cell->group[b]);
#else /* VIP_NA_MODE */
				if (b == bitMapIdx && !matchFound)
          cell->group[b] = currId;
#endif
			}

#ifdef VIP_DO_MODE
      currId = currId << VIP_CMOV(!matchFound, (VIP_ENCUINT64)1, (VIP_ENCUINT64)0);
#else /* VIP_NA_MODE */
      currId = currId << (!matchFound ? 1 : 0);
#endif

			matchFound = false;
		}
	}

	VIP_Enc_Group_t targetGr = initGroup;

	// Reverse pass
	for (int i = M - 1; i >= 0; i--)
	{
		for (int j = N - 1; j >= 0; j--)
		{
			node *cell = &(struct_mat[i][j]);
			VIP_Enc_Group_t commonGroup = cell->group;

			// Reverse read pass
			for (int k = 0; k < 8; k++)
			{
				if (SAFELOC(i + row[k], j + col[k]))
				{
					node *adjCell = &(struct_mat[i + row[k]][j + col[k]]);

					// Check if current position matches with the cell
					VIP_ENCBOOL match = (mat[i][j] == mat[i + row[k]][j + col[k]]);

					for (int b = 0; b < BITMAPS; b++)
					{
#ifdef VIP_DO_MODE
						commonGroup[b] = VIP_CMOV(match, combine(commonGroup[b], (adjCell->group)[b]), commonGroup[b]);
#else /* VIP_NA_MODE */
						if (match)
              commonGroup[b] = combine(commonGroup[b], (adjCell->group)[b]);
#endif
					}
				}
			}

			// Reverse write pass
			for (int k = 0; k <= 8; k++)
			{
				if (SAFELOC(i + row[k], j + col[k]))
				{
					node *adjCell = &(struct_mat[i + row[k]][j + col[k]]);

					VIP_ENCBOOL match = (mat[i][j] == mat[i + row[k]][j + col[k]]);

					VIP_ENCBOOL _istarget = (x == i + row[k]) && (y == j + col[k]);
					for (int b = 0; b < BITMAPS; b++)
					{
#ifdef VIP_DO_MODE
						(adjCell->group)[b] = VIP_CMOV(match, commonGroup[b], (adjCell->group)[b]);
						// Oblivious access to get target group
						targetGr[b] = VIP_CMOV(_istarget, (adjCell->group)[b], targetGr[b]);
#else /* VIP_NA_MODE */
						if (match)
              (adjCell->group)[b] = commonGroup[b];
						// Oblivious access to get target group
						if (_istarget)
              targetGr[b] = (adjCell->group)[b];
#endif
					}
				}
			}
		}
	}

	// Target Group Update pass: This pass is required for correctness [Verified - MZD]
	for (int ix = 0; ix < M; ix++)
	{
		for (int iy = 0; iy < N; iy++)
		{

			node *cell = &struct_mat[ix][iy];

			VIP_ENCBOOL cond = haveIntersection(cell->group, targetGr);
			// if group contains target group
			for (int b = 0; b < BITMAPS; b++)
			{
#ifdef VIP_DO_MODE
				targetGr[b] = VIP_CMOV(cond, combine(cell->group[b], targetGr[b]), targetGr[b]);
#else /* VIP_NA_MODE */
				if (cond)
          targetGr[b] = combine(cell->group[b], targetGr[b]);
#endif
			}
		}
	}

	// Coloring pass
	for (int i = 0; i < M; i++)
	{
		for (int j = 0; j < N; j++)
		{
			VIP_ENCBOOL flood = haveIntersection(targetGr, struct_mat[i][j].group);
#ifdef VIP_DO_MODE
			mat[i][j] = VIP_CMOV(flood, replacement, mat[i][j]);
#else /* VIP_NA_MODE */
			if (flood)
        mat[i][j] = replacement;
#endif
		}
	}
}


void printMatrix(VIP_ENCCHAR mat[M][N])
{
  for (int i = 0; i < M; i++)
  {
    for (int j = 0; j < N; j++)
    {
      cout << setw(3) << VIP_DEC(mat[i][j]);
    }
    cout << endl;
  }
}

int main()
{

  mysrand(0);

  VIP_ENCINT x = 3, y = 9;
  VIP_ENCCHAR replacement = 'C';


  cout << "\nBEFORE flooding `" << VIP_DEC(replacement) << "' @ "
       << "(" << VIP_DEC(x) << "," << VIP_DEC(y) << "):\n";
  printMatrix(mat);
  {
    Stopwatch start("VIP-bench runtime: ");
    floodfill(mat, x, y, replacement);
  }
  cout << "\nAFTER:" << endl;
  printMatrix(mat);
}

