#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"

#define NUM_POINTS 100
#define NUM_ITERATIONS 500
#define DIST_THRESHOLD 1.0

typedef struct
{
  fp64e_t x;
  fp64e_t y;
} Point;

static fp64e_t
line_distance(Point p, fp64e_t m, fp64e_t b)
{
  return mojov_abs(m * p.x - p.y + b) / mojov_sqrt(m * m + fp64e_t(1.0));
}

static void
ransac_line_fitting(Point points[], uint64e_t numPoints,
                    fp64e_t *best_m, fp64e_t *best_b,
                    uint64e_t *best_inlier_count)
{
  int numPointsPlain = (int)numPoints.decrypt();
  *best_inlier_count = 0;

  for (int iter = 0; iter < NUM_ITERATIONS; iter++)
  {
    int idx1 = libmin_rand() % numPointsPlain;
    int idx2 = libmin_rand() % numPointsPlain;
    while (idx2 == idx1)
      idx2 = libmin_rand() % numPointsPlain;

    Point p1 = points[idx1];
    Point p2 = points[idx2];

    fp64e_t dx = p2.x - p1.x;
    uint64e_t valid_model = (mojov_abs(dx) >= fp64e_t(1e-6));
    fp64e_t safe_dx = cmov(valid_model, dx, fp64e_t(1.0));
    fp64e_t m = (p2.y - p1.y) / safe_dx;
    fp64e_t b = p1.y - m * p1.x;

    uint64e_t inlierCount = 0;
    for (int i = 0; i < numPointsPlain; i++)
    {
      uint64e_t is_inlier = (line_distance(points[i], m, b) < fp64e_t(DIST_THRESHOLD));
      inlierCount += (is_inlier & valid_model);
    }

    uint64e_t is_better = (inlierCount > *best_inlier_count);
    *best_inlier_count = cmov(is_better, inlierCount, *best_inlier_count);
    *best_m = cmov(is_better, m, *best_m);
    *best_b = cmov(is_better, b, *best_b);
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

  libmin_srand(42);

  Point points[NUM_POINTS];

  int inlierCount = NUM_POINTS / 2;
  for (int i = 0; i < inlierCount; i++)
  {
    double x = ((double)i / inlierCount) * 50.0;
    double noise = ((double)libmin_rand() / RAND_MAX - 0.5) * 2.0;
    points[i].x = fp64e_t(x);
    points[i].y = fp64e_t(2 * x + 1 + noise);
  }

  for (int i = inlierCount; i < NUM_POINTS; i++)
  {
    points[i].x = fp64e_t(((double)libmin_rand() / RAND_MAX) * 50.0);
    points[i].y = fp64e_t(((double)libmin_rand() / RAND_MAX) * 100.0);
  }

  fp64e_t best_m = fp64e_t(0.0), best_b = fp64e_t(0.0);
  uint64e_t best_inlier_count = 0;
  uint64e_t num_points = NUM_POINTS;

  ransac_line_fitting(points, num_points, &best_m, &best_b, &best_inlier_count);

  libmin_printf("RANSAC estimated line: y = %f * x + %f\n", best_m.decrypt(), best_b.decrypt());
  libmin_printf("Number of inliers: %d / %d\n", (int)best_inlier_count.decrypt(), NUM_POINTS);

  libmin_success();
  return 0;
}
