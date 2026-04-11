#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"

// sizes support 10 (default), 15, 20, 25, 32, 45
// bitmaps needed 1          , 1,  1,  2,  3,  5
#define SIZE 10
#define BITMAPS 1

// `M × N` matrix
#define M SIZE
#define N SIZE

typedef uint64e_t Enc_Group_t[BITMAPS];
Enc_Group_t initGroup = { 0 }; //{BITMAPS x 0}
struct node
{
  Enc_Group_t group = { 0 };
};

// matrix showing portion of the screen having different colors
uint8e_t mat[M][N];
uint8e_t _mat[M][N] =
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
#endif
};

inline uint64e_t haveIntersection(Enc_Group_t bin1, Enc_Group_t bin2)
{
  uint64e_t intersect = false;
  for (int i = 0; i < BITMAPS; i++)
  {
    intersect = intersect || ((bin1[i] & bin2[i]) != 0);
  }

  return intersect;
}
inline uint64e_t combine(uint64e_t bin1, uint64e_t bin2)
{
  return bin1 | bin2;
}

void floodfill(uint8e_t mat[M][N], int64e_t x, int64e_t y, uint8e_t replacement)
{
  int row[] = {-1, -1, -1, 0, 0, 1, 1, 1, 0};
  int col[] = {-1, 0, 1, -1, 1, -1, 0, 1, 0};
  node struct_mat[M][N];
  uint64e_t currId = 1;
  int64e_t bitMapIdx = 0;

#define SAFELOC(X, Y) ((X) >= 0 && (X) < M && (Y) >= 0 && (Y) < N)

  uint64e_t matchFound = false;

  // Forward pass
  for (int i = 0; i < M; i++)
  {
    for (int j = 0; j < N; j++)
    {

      node *cell = &(struct_mat[i][j]);
      Enc_Group_t commonGroup[0] = cell->group[0];

      // Forward read pass
      for (int k = 0; k < 4; k++)
      {

        if (SAFELOC(i + row[k], j + col[k]))
        {
          node *adjCell = &(struct_mat[i + row[k]][j + col[k]]);

          uint64e_t match = (mat[i][j] == mat[i + row[k]][j + col[k]]);

          matchFound = match || matchFound;

          for (int b = 0; b < BITMAPS; b++)
            commonGroup[b] = cmov(match, combine(commonGroup[b], (adjCell->group)[b]), commonGroup[b]);
        }
      }

      cell->group = commonGroup;

      // Forward write pass
      for (int k = 0; k < 4; k++)
      {
        if (SAFELOC(i + row[k], j + col[k]))
        {
          node *adjCell = &(struct_mat[i + row[k]][j + col[k]]);

          uint64e_t match = (mat[i][j] == mat[i + row[k]][j + col[k]]);

          for (int b = 0; b < BITMAPS; b++)
            (adjCell->group)[b] = cmov(match, (commonGroup)[b], (adjCell->group)[b]);
        }
      }

      uint64e_t update_current_id = !matchFound && (currId == 0) && (bitMapIdx + 1 < BITMAPS);

      currId = cmov(update_current_id, (uint64e_t)1, currId);
      // currId = currId + (((currId + 1) << ((((-currId) | currId) >> (64 - 1)) & 1)) & 1);

      bitMapIdx += cmov(update_current_id, (int64e_t)1, (int64e_t)0);

      for (int b = 0; b < BITMAPS; b++)
        cell->group[b] = cmov(b == bitMapIdx && !matchFound, currId, cell->group[b]);

      currId = currId << cmov(!matchFound, (uint64e_t)1, (uint64e_t)0);
      matchFound = false;
    }
  }

  Enc_Group_t targetGr = { 0 };

  // Reverse pass
  for (int i = M - 1; i >= 0; i--)
  {
    for (int j = N - 1; j >= 0; j--)
    {
      node *cell = &(struct_mat[i][j]);
      Enc_Group_t commonGroup = cell->group;

      // Reverse read pass
      for (int k = 0; k < 8; k++)
      {
        if (SAFELOC(i + row[k], j + col[k]))
        {
          node *adjCell = &(struct_mat[i + row[k]][j + col[k]]);

          // Check if current position matches with the cell
          uint64e_t match = (mat[i][j] == mat[i + row[k]][j + col[k]]);

          for (int b = 0; b < BITMAPS; b++)
            commonGroup[b] = cmov(match, combine(commonGroup[b], (adjCell->group)[b]), commonGroup[b]);
        }
      }

      // Reverse write pass
      for (int k = 0; k <= 8; k++)
      {
        if (SAFELOC(i + row[k], j + col[k]))
        {
          node *adjCell = &(struct_mat[i + row[k]][j + col[k]]);

          uint64e_t match = (mat[i][j] == mat[i + row[k]][j + col[k]]);

          uint64e_t _istarget = (x == i + row[k]) && (y == j + col[k]);
          for (int b = 0; b < BITMAPS; b++)
          {
            (adjCell->group)[b] = cmov(match, commonGroup[b], (adjCell->group)[b]);
            // Oblivious access to get target group
            targetGr[b] = cmov(_istarget, (adjCell->group)[b], targetGr[b]);
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

      uint64e_t cond = haveIntersection(cell->group, targetGr);
      // if group contains target group
      for (int b = 0; b < BITMAPS; b++)
        targetGr[b] = cmov(cond, combine(cell->group[b], targetGr[b]), targetGr[b]);
    }
  }

  // Coloring pass
  for (int i = 0; i < M; i++)
  {
    for (int j = 0; j < N; j++)
    {
      uint64e_t flood = haveIntersection(targetGr, struct_mat[i][j].group);
      mat[i][j] = cmov(flood, replacement, mat[i][j]);
    }
  }
}


void printMatrix(uint8e_t mat[M][N])
{
  for (int i = 0; i < M; i++)
  {
    for (int j = 0; j < N; j++)
    {
      cout << setw(3) << (mat[i][j]).decrypt();
    }
    cout << endl;
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

  // initialize the encrypted input data
  for (int i=0; i<M; i++)
    for (int j=0; j<N; j++)
      mat[i][j] = _mat[i][j];

  int64e_t x = 3, y = 9;
  uint8e_t replacement = 'C';

  cout << "\nBEFORE flooding `" << replacement.decrypt() << "' @ "
       << "(" << x.decrypt() << "," << y.decrypt() << "):\n";
  printMatrix(mat);
  {
    // Stopwatch start("VIP-bench runtime: ");
    floodfill(mat, x, y, replacement);
  }
  cout << "\nAFTER:" << endl;
  printMatrix(mat);

  libmin_success();
  return 0;
}
