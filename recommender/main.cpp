#include <iostream>
#include <ctime>

#include "../config.h"
#include "parser.hpp"
#include "rating.hpp"
#include "predictor.hpp"

int
main(int argc, char *argv[])
{
    char *filename = argv[1];
    size_t K = std::atoi(argv[2]);
    double eta;
    double lambda;
    size_t iterations = 0;

    // initialize RNG
    mysrand(42);

    // Read arguments
    if (argc > 3) {
        eta = std::atof(argv[3]);
        lambda = std::atof(argv[4]);
    }

    // Read optional iterations argument
    if (argc == 6) {
        iterations = std::atoi(argv[5]);
    }

    // Parser initialization
    Parser *parser = new Parser(filename, ',');

    // Read CSV file
    std::cout << "Parsing data file: " << filename << std::endl;
    parser->readCsv();

    // Split train test ratings
    std::cout << "Split training/testing datasets" << std::endl;
    parser->splitTrainTestRatings(0.8, 3);

    // Ratings matrix
    Predictor *predictor = new Predictor(parser->getTrainRatings(),
                                         parser->getTestRatings(),
                                         parser->getUserNb(),
                                         parser->getMovieNb());

    // Fully specified argument prediction matrix
    if (argc > 3) {
        std::cout << "====== Matrix factorization ======" << std::endl;
        std::cout << "K = " << K
                  << ", eta = " << eta
                  << ", lambda = " << lambda
                  << ", iterations = " << iterations
                  << std::endl;
        std::clock_t start = clock();
        {
          Stopwatch s("VIP_Bench Runtime");
          predictor->predictionMatrix(K, eta, lambda, iterations);
        }
        double duration = (double)(clock() - start) / CLOCKS_PER_SEC;

        // Training MAE
        std::cout << "Training MAE = "
                  << VIP_DEC(predictor->trainingMeanAbsoluteError())
                  << std::endl;

        // Testing MAE
        std::cout << "Testing MAE = "
                  << VIP_DEC(predictor->testingMeanAbsoluteError())
                  << std::endl;

        // Write results into text file
        parser->writeResultsFile("results.txt", predictor);

        // Factorization duration
        std::cerr << "Factorization duration: " << duration << " s" << std::endl;

    // Prediction matrix Cross-Validation
    } else {
        double etaArr[] = { 1e-4, 1e-3, 1e-2 };
        double lambdaArr[] = { 1e-3, 1e-2, 1e-1, 1, 1e1, 1e2, 1e3 };

        // Mean of MAE
        VIP_ENCDOUBLE mae;

        std::cout << "======== Matrix factorization cross-validation ========" << std::endl;
        for (double eta: etaArr) {
            for (double lambda: lambdaArr) {
                // Reset MAE
                mae = 0;

                for (int l = 0; l < 5; ++l) {
                    // Pick next test set
                    parser->splitTrainTestRatings(0.8, l);

                    // Compute prediction matrix
                    predictor->predictionMatrix(K, eta, lambda, 0);

                    // Compute MAE
                    mae = mae + predictor->testingMeanAbsoluteError();
                }

                // Normalize MAE
                mae = mae / 5;
                std::cout << "K = " << K
                          << ", eta = " << eta
                          << ", lambda = " << lambda
                          << ", iterations = " << iterations
                          << ", Train MAE = " << VIP_DEC(predictor->trainingMeanAbsoluteError())
                          << ", Test MAE = " << VIP_DEC(mae)
                          << std::endl;
            }
        }
    }

    // Free memory
    delete parser;
    delete predictor;

    // Exit success
    return 0;
}
