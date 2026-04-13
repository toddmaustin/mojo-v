#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"

// sizes support 10 (default), 15, 20, 25, 32, 45
#define SIZE 10
#define BITMAPS 1

// `M × N` matrix
#define M SIZE
#define N SIZE

struct node
{
  uint64e_t group[BITMAPS];
};

// matrix showing portion of the screen having different colors
int8e_t mat[M][N];
uint8_t _mat[M][N] =
{
#if SIZE == 10
  { 'Y', 'Y', 'Y', 'G', 'G', 'G', 'G', 'G', 'G', 'G' },
  { 'Y', 'Y', 'Y', 'Y', 'Y', 'Y', 'G', 'X', 'X', 'X' },
  { 'G', 'X', 'G', 'G', 'G', 'G', 'G', 'X', 'X', 'X' },
  { 'W', 'X', 'X', 'W', 'W', 'G', 'G', 'G', 'G', 'X' },
  { 'W', 'R', 'R', 'R', 'R', 'R', 'G', 'X', 'X', 'X' },
  { 'W', 'W', 'W', 'R', 'R', 'G', 'G', 'X', 'X', 'X' },
  { 'W', 'B', 'W', 'R', 'R', 'R', 'R', 'R', 'R', 'X' },
  { 'W', 'B', 'B', 'B', 'B', 'R', 'R', 'X', 'X', 'X' },
  { 'W', 'B', 'B', 'X', 'B', 'B', 'B', 'B', 'X', 'X' },
  { 'W', 'B', 'B', 'X', 'X', 'X', 'X', 'X', 'X', 'X' }
#endif
};

inline int8e_t
haveIntersection(uint64e_t bin1[BITMAPS], uint64e_t bin2[BITMAPS])
{
  int8e_t intersect = 0;
  for (int i = 0; i < BITMAPS; i++)
  {
    intersect = intersect || ((bin1[i] & bin2[i]) != 0);
  }

  return intersect;
}

inline uint64e_t
combine(uint64e_t bin1, uint64e_t bin2)
{
  return bin1 | bin2;
}

void
floodfill(int8e_t _mat[M][N], int64e_t x, int64e_t y, int8e_t replacement)
{
  int row[] = { -1, -1, -1, 0, 0, 1, 1, 1, 0 };
  int col[] = { -1, 0, 1, -1, 1, -1, 0, 1, 0 };
  node struct_mat[M][N];
  uint64e_t currId = 1;
  int64e_t bitMapIdx = 0;

#define SAFELOC(X, Y) ((X) >= 0 && (X) < M && (Y) >= 0 && (Y) < N)

  int8e_t matchFound = 0;

  // initialize groups (encrypted globals cannot be initialized statically)
  for (int i = 0; i < M; i++)
  {
    for (int j = 0; j < N; j++)
    {
      for (int b = 0; b < BITMAPS; b++)
      {
        struct_mat[i][j].group[b] = 0;
      }
    }
  }

  // Forward pass
  for (int i = 0; i < M; i++)
  {
    for (int j = 0; j < N; j++)
    {
      node* cell = &(struct_mat[i][j]);
      uint64e_t commonGroup[BITMAPS];

      for (int b = 0; b < BITMAPS; b++)
      {
        commonGroup[b] = cell->group[b];
      }

      // Forward read pass
      for (int k = 0; k < 4; k++)
      {
        if (SAFELOC(i + row[k], j + col[k]))
        {
          node* adjCell = &(struct_mat[i + row[k]][j + col[k]]);

          int8e_t match = (_mat[i][j] == _mat[i + row[k]][j + col[k]]);

          matchFound = match || matchFound;

          for (int b = 0; b < BITMAPS; b++)
          {
            commonGroup[b] = cmov(match, combine(commonGroup[b], (adjCell->group)[b]), commonGroup[b]);
          }
        }
      }

      for (int b = 0; b < BITMAPS; b++)
      {
        cell->group[b] = commonGroup[b];
      }

      // Forward write pass
      for (int k = 0; k < 4; k++)
      {
        if (SAFELOC(i + row[k], j + col[k]))
        {
          node* adjCell = &(struct_mat[i + row[k]][j + col[k]]);

          int8e_t match = (_mat[i][j] == _mat[i + row[k]][j + col[k]]);

          for (int b = 0; b < BITMAPS; b++)
          {
            (adjCell->group)[b] = cmov(match, (commonGroup)[b], (adjCell->group)[b]);
          }
        }
      }

      int8e_t updateCurrentId = !matchFound && (currId == 0) && (bitMapIdx + 1 < BITMAPS);

      currId = cmov(updateCurrentId, (uint64e_t)1, currId);
      bitMapIdx += (int64e_t)cmov(updateCurrentId, (int64e_t)1, (int64e_t)0);

      for (int b = 0; b < BITMAPS; b++)
      {
        cell->group[b] = cmov((b == bitMapIdx) && !matchFound, currId, cell->group[b]);
      }

      currId = currId << cmov(!matchFound, (uint64e_t)1, (uint64e_t)0);

      matchFound = 0;
    }
  }

  uint64e_t targetGr[BITMAPS];
  for (int b = 0; b < BITMAPS; b++)
  {
    targetGr[b] = 0;
  }

  // Reverse pass
  for (int i = M - 1; i >= 0; i--)
  {
    for (int j = N - 1; j >= 0; j--)
    {
      node* cell = &(struct_mat[i][j]);
      uint64e_t commonGroup[BITMAPS];

      for (int b = 0; b < BITMAPS; b++)
      {
        commonGroup[b] = cell->group[b];
      }

      // Reverse read pass
      for (int k = 0; k < 8; k++)
      {
        if (SAFELOC(i + row[k], j + col[k]))
        {
          node* adjCell = &(struct_mat[i + row[k]][j + col[k]]);

          int8e_t match = (_mat[i][j] == _mat[i + row[k]][j + col[k]]);

          for (int b = 0; b < BITMAPS; b++)
          {
            commonGroup[b] = cmov(match, combine(commonGroup[b], (adjCell->group)[b]), commonGroup[b]);
          }
        }
      }

      // Reverse write pass
      for (int k = 0; k <= 8; k++)
      {
        if (SAFELOC(i + row[k], j + col[k]))
        {
          node* adjCell = &(struct_mat[i + row[k]][j + col[k]]);

          int8e_t match = (_mat[i][j] == _mat[i + row[k]][j + col[k]]);

          int8e_t _istarget = (x == i + row[k]) && (y == j + col[k]);
          for (int b = 0; b < BITMAPS; b++)
          {
            (adjCell->group)[b] = cmov(match, commonGroup[b], (adjCell->group)[b]);
            targetGr[b] = cmov(_istarget, (adjCell->group)[b], targetGr[b]);
          }
        }
      }
    }
  }

  // Target Group Update pass
  for (int ix = 0; ix < M; ix++)
  {
    for (int iy = 0; iy < N; iy++)
    {
      node* cell = &struct_mat[ix][iy];

      int8e_t cond = haveIntersection(cell->group, targetGr);
      for (int b = 0; b < BITMAPS; b++)
      {
        targetGr[b] = cmov(cond, combine(cell->group[b], targetGr[b]), targetGr[b]);
      }
    }
  }

  // Coloring pass
  for (int i = 0; i < M; i++)
  {
    for (int j = 0; j < N; j++)
    {
      int8e_t flood = haveIntersection(targetGr, struct_mat[i][j].group);
      _mat[i][j] = cmov(flood, replacement, _mat[i][j]);
    }
  }
}

void
printMatrix(int8e_t _mat[M][N])
{
  for (int i = 0; i < M; i++)
  {
    for (int j = 0; j < N; j++)
    {
      libmin_printf(" %c ", _mat[i][j].decrypt());
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
  libmin_srand(0);

  // initialize character matrix (encrypted globals cannot be initialized statically)
  for (int i = 0; i < M; i++)
  {
    for (int j = 0; j < N; j++)
    {
      mat[i][j] = _mat[i][j];
    }
  }

  int64e_t x = 3;
  int64e_t y = 9;
  int8e_t replacement = 'C';

  libmin_printf("\nBEFORE flooding `%c' @ (%d,%d):\n", replacement.decrypt(), x.decrypt(), y.decrypt());
  printMatrix(mat);

  floodfill(mat, x, y, replacement);

  libmin_printf("\nAFTER:\n");
  printMatrix(mat);

  libmin_success();
  return 0;
}
