#include "parser.hpp"
#include "rating.hpp"
#include "predictor.hpp"

#include <iostream>
#include <ctime>

int main(int argc, char *argv[]) {
    char *filename = argv[1];
    size_t K = std::atoi(argv[2]);
    double eta;
    double lambda;

    // Read arguments
    if (argc > 3) {
        eta = std::atof(argv[3]);
        lambda = std::atof(argv[4]);
    }

    // Parser initialization
    Parser *parser = new Parser(filename, ',');

    // Read CSV file
    std::cout << "Parsing data file: " << filename << std::endl;
    parser->readCsv();

    // Split train test ratings
    std::cout << "Split training/testing datasets" << std::endl;
    parser->splitTrainTestRatings(0.8);

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
                  << std::endl;
        std::clock_t start = clock();
        predictor->predictionMatrix(K, eta, lambda);
        double duration = (double)(clock() - start) / CLOCKS_PER_SEC;

        // Training MAE
        std::cout << "Training MAE = "
                  << predictor->trainingMeanAbsoluteError()
                  << std::endl;

        // Testing MAE
        std::cout << "Testing MAE = "
                  << predictor->testingMeanAbsoluteError()
                  << std::endl;

        // Write results into text file
        parser->writeResultsFile("results.txt", predictor);

        // Factorization duration
        std::cout << "Factorization duration: " << duration << " s" << std::endl;

    // Prediction matrix Cross-Validation
    } else {
        double etaArr[] = { 1e-5, 1e-4, 1e-3, 1e-2 };
        double lambdaArr[] = { 1e-3, 1e-2, 1e-1, 1, 1e1, 1e2, 1e3 };

        // Mean of MAE
        double mae;

        std::cout << "======== Matrix factorization cross-validation ========" << std::endl;
        for (double eta: etaArr) {
            for (double lambda: lambdaArr) {
                // Reset MAE
                mae = 0;

                for (int l = 0; l < 1; ++l) {
                    // Compute prediction matrix
                    predictor->predictionMatrix(K, eta, lambda);

                    // Compute MAE
                    mae += predictor->testingMeanAbsoluteError();
                }

                // Normalize MAE
                mae /= 1;
                std::cout << "K = " << K
                          << ", eta = " << eta
                          << ", lambda = " << lambda
                          << ", Train MAE = " << predictor->trainingMeanAbsoluteError()
                          << ", Test MAE = " << mae
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
