#ifndef PREDICTOR_HPP
#define PREDICTOR_HPP

#include "rating.hpp"

#include <iostream>
#include <vector>

class Predictor {

public:
    Predictor(const std::vector<Rating*>& training,
              const std::vector<Rating*>& testing,
              size_t userNb, size_t movieNb);

    ~Predictor();

    VIP_ENCDOUBLE** genRandomMatrix(size_t n, size_t m);

    void matrixFactorization(
            VIP_ENCDOUBLE **U, VIP_ENCDOUBLE **M,
            size_t K, double eta, double lambda,
            size_t iterations);

    VIP_ENCDOUBLE meanAbsoluteError(VIP_ENCDOUBLE **U, VIP_ENCDOUBLE **M, size_t K);

    bool convergeEnough(double tolerance, double mae, double *oldMae, int size);

    void predictionMatrix(size_t K, double eta, double lambda, size_t iterations);

    VIP_ENCDOUBLE predict(size_t user, size_t movie);

    VIP_ENCDOUBLE trainingMeanAbsoluteError();

    VIP_ENCDOUBLE testingMeanAbsoluteError();

private:
    VIP_ENCDOUBLE** ratings;
    VIP_ENCDOUBLE** predictions;

    size_t ratingNb;
    size_t userNb;
    size_t movieNb;

    std::vector<Rating*> training;
    std::vector<Rating*> testing;
};

#endif
