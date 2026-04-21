#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"

#define INPUTS 2
#define HIDDEN 3
#define SAMPLES 4
#define EPOCHS 80

static const double training_inputs[SAMPLES][INPUTS] = {
  {0.0, 0.0},
  {0.0, 1.0},
  {1.0, 0.0},
  {1.0, 1.0}
};

// OR function labels.
static const double training_labels[SAMPLES] = {0.0, 1.0, 1.0, 1.0};

static inline fp64e_t
relu(fp64e_t x)
{
  return cmov(x > (fp64e_t)0.0, x, (fp64e_t)0.0);
}

static inline fp64e_t
relu_prime(fp64e_t x)
{
  return cmov(x > (fp64e_t)0.0, (fp64e_t)1.0, (fp64e_t)0.0);
}

// Fast sigmoid approximation: 0.5 + x/(2*(1+|x|)).
static inline fp64e_t
sigmoid_approx(fp64e_t x)
{
  fp64e_t abs_x = cmov(x >= (fp64e_t)0.0, x, -x);
  return (fp64e_t)0.5 + x / ((fp64e_t)2.0 * ((fp64e_t)1.0 + abs_x));
}

static inline fp64e_t
sigmoid_approx_prime_from_output(fp64e_t y)
{
  return y * ((fp64e_t)1.0 - y);
}

int
main(void)
{
  if (mojov_configure_kmsm_from_dc_fast() != 0)
    return -1;

  if (mojov_enable_and_verify() != 0)
    return -1;

  if (debug_context(SIMON128_KEY, CONTRACT_SIG) != 0)
    return -1;

  // Encrypted model parameters. Initialized in main() to satisfy Mojo-V
  // restriction against encrypted static initialization.
  fp64e_t w1[INPUTS][HIDDEN];
  fp64e_t b1[HIDDEN];
  fp64e_t w2[HIDDEN];
  fp64e_t b2;

  w1[0][0] = (fp64e_t)0.40; w1[0][1] = (fp64e_t)-0.30; w1[0][2] = (fp64e_t)0.20;
  w1[1][0] = (fp64e_t)-0.10; w1[1][1] = (fp64e_t)0.50; w1[1][2] = (fp64e_t)0.30;
  b1[0] = (fp64e_t)0.00; b1[1] = (fp64e_t)-0.10; b1[2] = (fp64e_t)0.10;

  w2[0] = (fp64e_t)0.25; w2[1] = (fp64e_t)-0.20; w2[2] = (fp64e_t)0.45;
  b2 = (fp64e_t)-0.05;

  fp64e_t lr = (fp64e_t)0.20;

  for (unsigned epoch = 0; epoch < EPOCHS; ++epoch)
  {
    fp64e_t epoch_loss = (fp64e_t)0.0;

    for (unsigned s = 0; s < SAMPLES; ++s)
    {
      fp64e_t x[INPUTS];
      x[0] = (fp64e_t)training_inputs[s][0];
      x[1] = (fp64e_t)training_inputs[s][1];
      fp64e_t target = (fp64e_t)training_labels[s];

      fp64e_t z1[HIDDEN];
      fp64e_t a1[HIDDEN];
      for (unsigned j = 0; j < HIDDEN; ++j)
      {
        z1[j] = b1[j];
        for (unsigned i = 0; i < INPUTS; ++i)
          z1[j] += x[i] * w1[i][j];
        a1[j] = relu(z1[j]);
      }

      fp64e_t z2 = b2;
      for (unsigned j = 0; j < HIDDEN; ++j)
        z2 += a1[j] * w2[j];
      fp64e_t y = sigmoid_approx(z2);

      fp64e_t err = y - target;
      epoch_loss += err * err;

      fp64e_t delta2 = err * sigmoid_approx_prime_from_output(y);

      fp64e_t delta1[HIDDEN];
      for (unsigned j = 0; j < HIDDEN; ++j)
        delta1[j] = (delta2 * w2[j]) * relu_prime(z1[j]);

      for (unsigned j = 0; j < HIDDEN; ++j)
        w2[j] -= lr * (delta2 * a1[j]);
      b2 -= lr * delta2;

      for (unsigned i = 0; i < INPUTS; ++i)
      {
        for (unsigned j = 0; j < HIDDEN; ++j)
          w1[i][j] -= lr * (delta1[j] * x[i]);
      }
      for (unsigned j = 0; j < HIDDEN; ++j)
        b1[j] -= lr * delta1[j];
    }

    if ((epoch % 20) == 0 || (epoch + 1 == EPOCHS))
      libmin_printf("tiny-NN train epoch=%u mse=%.6lf\n", epoch, (epoch_loss / (fp64e_t)SAMPLES).decrypt());
  }

  unsigned correct = 0;
  for (unsigned s = 0; s < SAMPLES; ++s)
  {
    fp64e_t x0 = (fp64e_t)training_inputs[s][0];
    fp64e_t x1 = (fp64e_t)training_inputs[s][1];

    fp64e_t h[HIDDEN];
    for (unsigned j = 0; j < HIDDEN; ++j)
      h[j] = relu(x0 * w1[0][j] + x1 * w1[1][j] + b1[j]);

    fp64e_t y = b2;
    for (unsigned j = 0; j < HIDDEN; ++j)
      y += h[j] * w2[j];
    y = sigmoid_approx(y);

    uint64_t pred = (y > (fp64e_t)0.5).decrypt();
    uint64_t expected = (uint64_t)training_labels[s];
    correct += (pred == expected) ? 1u : 0u;

    libmin_printf("tiny-NN infer x=(%.0lf,%.0lf) y=%.6lf pred=%lu expected=%lu\n",
      training_inputs[s][0], training_inputs[s][1], y.decrypt(), pred, expected);
  }

  libmin_printf("tiny-NN summary: %u/%u correct\n", correct, SAMPLES);

  if (correct != SAMPLES)
    return -1;

  libmin_success();
  return 0;
}
