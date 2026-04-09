#include <omp.h>
#include <iostream>
#include <cmath>

#include "../config.h"
#include "mathlib.h"
#include "predictor.hpp"

Predictor::Predictor(const std::vector<Rating *>& training,
                     const std::vector<Rating *>& testing,
                     size_t userNb, size_t movieNb) {
    this->training = training;
    this->testing = testing;
    this->userNb = userNb;
    this->movieNb = movieNb;

    // Prediction matrix allocation
    this->predictions = new VIP_ENCDOUBLE*[this->userNb];
    for (size_t i = 0; i < this->userNb; ++i) {
        this->predictions[i] = new VIP_ENCDOUBLE[this->movieNb];
    }
}

Predictor::~Predictor() {
    for (unsigned i = 0; i < this->userNb; ++i) {
        delete [] this->predictions[i];
    }
    delete [] this->predictions;
}

VIP_ENCDOUBLE** Predictor::genRandomMatrix(size_t n, size_t m) {
    // Matrix allocation
    VIP_ENCDOUBLE **mat = new VIP_ENCDOUBLE*[n];
    for (size_t i = 0; i < n; ++i) {
        mat[i] = new VIP_ENCDOUBLE[m];
    }

    // Fill matrix with random values
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < m; ++j) {
            mat[i][j] = (double)myrand() / RAND_MAX * 5;
        }
    }

    // Return the matrix
    return mat;
}

void Predictor::matrixFactorization(
        VIP_ENCDOUBLE **U, VIP_ENCDOUBLE **M,
        size_t K, double eta, double lambda,
        size_t iterations) {
    // Initialize MAE
    VIP_ENCDOUBLE mae = -1;
    double oldMae[3] = { 0, 0, 0 };

    if (iterations == 0) {
        // Iterate until MAE converge enough
        while (!this->convergeEnough(1e-5, VIP_DEC(mae), oldMae, 3)) {
            for (Rating* rating: this->training) {
                size_t i = rating->getUser() - 1;
                size_t j = rating->getMovie() - 1;

                // Matrix product
                VIP_ENCDOUBLE dotProd = 0;
                for (size_t k = 0; k < K; ++k) {
                    dotProd = dotProd + U[i][k] * M[k][j];
                }

                // ij term error
                VIP_ENCDOUBLE error = rating->getMark() - dotProd;

                // Stochastic gradient descent iteration
                for (size_t k = 0; k < K; ++k) {
                    U[i][k] = U[i][k] + ((VIP_ENCDOUBLE)eta * (error * M[k][j] - (VIP_ENCDOUBLE)lambda * U[i][k]));
                    M[k][j] = M[k][j] + ((VIP_ENCDOUBLE)eta * (error * U[i][k] - (VIP_ENCDOUBLE)lambda * M[k][j]));
                }
            }

            // Compute the new MAE
            mae = this->meanAbsoluteError(U, M, K);
        }
    } else {
        // Iterate a specified amount
        for (size_t iteration = 0; iteration < iterations; ++iteration) {
            for (Rating* rating: this->training) {
                size_t i = rating->getUser() - 1;
                size_t j = rating->getMovie() - 1;

                // Matrix product
                VIP_ENCDOUBLE dotProd = 0;
                for (size_t k = 0; k < K; ++k) {
                    dotProd = dotProd + (U[i][k] * M[k][j]);
                }

                // ij term error
                VIP_ENCDOUBLE error = rating->getMark() - dotProd;

                // Stochastic gradient descent iteration
                for (size_t k = 0; k < K; ++k) {
                    U[i][k] = U[i][k] + ((VIP_ENCDOUBLE)eta * (error * M[k][j] - (VIP_ENCDOUBLE)lambda * U[i][k]));
                    M[k][j] = M[k][j] + ((VIP_ENCDOUBLE)eta * (error * U[i][k] - (VIP_ENCDOUBLE)lambda * M[k][j]));
                }
            }
        }
    }
}

bool Predictor::convergeEnough(double tolerance, double mae, double *oldMae, int size) {
    // Avoid NAN
    if (mae != mae) {
        return true;
    }

    // Avoid divergence
    if (mae > oldMae[0] && oldMae[0] > 0) {
        return true;
    }

    double diff = 0;
    for (int i = 0; i < size; ++i) {
        diff += std::abs(oldMae[i] - mae);
    }

    if (diff < tolerance) {
        return true;
    } else {
        for (int i = size - 1; i > 0; --i) {
            oldMae[i] = oldMae[i-1];
        }
        oldMae[0] = mae;
        return false;
    }
}

VIP_ENCDOUBLE Predictor::meanAbsoluteError(VIP_ENCDOUBLE **U, VIP_ENCDOUBLE **M, size_t K) {
    VIP_ENCDOUBLE error = 0;

    for (Rating* rating: this->testing) {
        size_t i = rating->getUser() - 1;
        size_t j = rating->getMovie() - 1;

        VIP_ENCDOUBLE dotProd = 0;
        for (size_t k = 0; k < K; ++k) {
            dotProd = dotProd + (U[i][k] * M[k][j]);
        }
        error = error + (myfabs(rating->getMark() - dotProd));
    }

    // Normalization
    error = error / this->testing.size();

    return error;
}

void Predictor::predictionMatrix(size_t K, double eta, double lambda, size_t iterations) {
    // Initialize User and Movie matrix
    VIP_ENCDOUBLE **U = this->genRandomMatrix(this->userNb, K);
    VIP_ENCDOUBLE **M = this->genRandomMatrix(K, this->movieNb);

    // Matrix factorization
    this->matrixFactorization(U, M, K, eta, lambda, iterations);

    // Compute predictions
    for (size_t i = 0; i < this->userNb; ++i) {
        for (size_t j = 0; j < this->movieNb; ++j) {
            VIP_ENCDOUBLE dotProd = 0;
            for (size_t k = 0; k < K; ++k) {
                dotProd = dotProd + (U[i][k] * M[k][j]);
            }
            this->predictions[i][j] = dotProd;
        }
    }

    // Free U memory
    for (size_t i = 0; i < this->userNb; ++i) {
        delete [] U[i];
    }
    delete [] U;

    // Free M memory
    for (size_t i = 0; i < K; ++i) {
        delete [] M[i];
    }
    delete [] M;
}

VIP_ENCDOUBLE Predictor::predict(size_t user, size_t movie) {
    return this->predictions[user][movie];
}

VIP_ENCDOUBLE Predictor::trainingMeanAbsoluteError() {
    VIP_ENCDOUBLE error = 0;

    for (Rating *rating: this->training) {
        size_t i = rating->getUser() - 1;
        size_t j = rating->getMovie() - 1;
        error = error + myfabs(rating->getMark() - this->predictions[i][j]);
    }

    // Normalization
    error = error / this->training.size();

    return error;
}

VIP_ENCDOUBLE Predictor::testingMeanAbsoluteError() {
    VIP_ENCDOUBLE error = 0;

    for (Rating *rating: this->testing) {
        size_t i = rating->getUser() - 1;
        size_t j = rating->getMovie() - 1;
        error = error + myfabs(rating->getMark() - this->predictions[i][j]);
    }

    // Normalization
    error = error / this->testing.size();

    return error;
}
