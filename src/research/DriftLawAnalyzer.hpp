#pragma once

#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cmath>
#include "DataExporter.hpp"

namespace collatz {
namespace research {

// ============================================================
// Module 20: Stopping-Time vs Drift Law Analyzer
//
// Tests the hypothesis that S(n) ≈ A + B * (log(n) / |mu_n|)
// where S(n) is the total stopping time, and mu_n is the
// trajectory-average drift: E[log(3/2^v2)].
// ============================================================

class DriftLawAnalyzer {
public:
    static void analyze(unsigned long long limit) {
        std::cout << "\n========================================================\n";
        std::cout << "Research Module 20: Stopping-Time vs Drift Law\n";
        std::cout << "Limit : " << limit << "\n";
        std::cout << "Hypothesis : S(n) ≈ A + B * (log(n) / |mu_n|)\n";
        std::cout << "========================================================\n\n";

        auto t0 = std::chrono::high_resolution_clock::now();

        const double LOG2 = std::log(2.0);
        const double LOG3 = std::log(3.0);

        std::vector<double> X; // Predictor: log(n) / |mu_n|
        std::vector<double> Y; // Actual: S(n)
        std::vector<double> mus; // Raw drift for CSV

        X.reserve(limit);
        Y.reserve(limit);
        mus.reserve(limit);

        for (unsigned long long n = 2; n <= limit; ++n) { // Start from 2
            unsigned long long current = n;
            int total_steps = 0;
            int odd_steps = 0;
            double sum_drift = 0.0;

            while (current != 1) {
                if (current % 2 == 1) {
                    current = 3 * current + 1;
                    total_steps++;
                    int v2 = 0;
                    while (current % 2 == 0 && current > 0) {
                        current /= 2;
                        v2++;
                        total_steps++;
                    }
                    sum_drift += (LOG3 - v2 * LOG2);
                    odd_steps++;
                } else {
                    current /= 2;
                    total_steps++;
                }
            }

            double mu_n = 0.0;
            double X_n = 0.0;

            if (odd_steps > 0) {
                mu_n = sum_drift / odd_steps;
                // Since the sequence reaches 1, mu_n must be negative overall.
                // We use |mu_n| to avoid negative values in the predictor.
                X_n = std::log((double)n) / std::abs(mu_n);
            } else {
                // If it's a power of 2, there are no odd steps.
                // S(n) = log2(n) = log(n)/log(2).
                // Effectively, drift is -log(2).
                mu_n = -LOG2;
                X_n = std::log((double)n) / LOG2;
            }

            X.push_back(X_n);
            Y.push_back((double)total_steps);
            mus.push_back(mu_n);
        }

        auto t1 = std::chrono::high_resolution_clock::now();
        long long sim_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

        // Perform 1D Linear Regression
        size_t N = X.size();
        double sum_x = 0, sum_y = 0;
        for (size_t i = 0; i < N; ++i) {
            sum_x += X[i];
            sum_y += Y[i];
        }
        double mean_x = sum_x / N;
        double mean_y = sum_y / N;

        double cov_xy = 0, var_x = 0, var_y = 0;
        for (size_t i = 0; i < N; ++i) {
            double dx = X[i] - mean_x;
            double dy = Y[i] - mean_y;
            cov_xy += dx * dy;
            var_x += dx * dx;
            var_y += dy * dy;
        }

        double B = cov_xy / var_x;
        double A = mean_y - B * mean_x;
        double r = cov_xy / std::sqrt(var_x * var_y);
        double R2 = r * r;

        // Calculate Errors
        double sum_abs_err = 0;
        double sum_sq_err = 0;
        for (size_t i = 0; i < N; ++i) {
            double predicted = A + B * X[i];
            double err = predicted - Y[i];
            sum_abs_err += std::abs(err);
            sum_sq_err += err * err;
        }
        double MAE = sum_abs_err / N;
        double RMSE = std::sqrt(sum_sq_err / N);

        std::cout << "--- Regression Results ---\n";
        std::cout << "  Predictor X_n : log(n) / |mu_n|\n";
        std::cout << "  Model         : S(n) = A + B * X_n\n\n";

        std::cout << "  Coefficients:\n";
        std::cout << "    A (Intercept) : " << std::fixed << std::setprecision(4) << A << "\n";
        std::cout << "    B (Slope)     : " << std::fixed << std::setprecision(4) << B << "\n\n";

        std::cout << "  Metrics:\n";
        std::cout << "    Correlation r : " << std::fixed << std::setprecision(4) << r << "\n";
        std::cout << "    R^2           : " << std::fixed << std::setprecision(4) << R2 << "\n";
        std::cout << "    MAE           : " << std::fixed << std::setprecision(4) << MAE << "\n";
        std::cout << "    RMSE          : " << std::fixed << std::setprecision(4) << RMSE << "\n\n";

        std::cout << "--- Scientific Interpretation ---\n";
        if (R2 > 0.95) {
            std::cout << "  SUCCESS! R^2 > 0.95.\n";
            std::cout << "  This formally bridges the size-growth law with the drift heuristic.\n";
            std::cout << "  Stopping time is essentially determined by the logarithmic size of the\n";
            std::cout << "  number divided by the average drift of its trajectory.\n\n";
        } else {
            std::cout << "  R^2 is not > 0.95. The hypothesis is weak.\n\n";
        }

        std::cout << "  Simulation time : " << sim_ms << " ms\n";

        // Export to CSV
        std::stringstream csv;
        csv << "n,mu_n,predictor_X,actual_S,predicted_S,error\n";
        for (size_t i = 0; i < N; ++i) {
            double predicted = A + B * X[i];
            csv << (i + 2) << "," << mus[i] << "," << X[i] << "," << Y[i] << "," << predicted << "," << (predicted - Y[i]) << "\n";
        }
        DataExporter::export_csv(
            "drift_law_limit_" + std::to_string(limit) + ".csv",
            csv.str());
    }
};

} // namespace research
} // namespace collatz
