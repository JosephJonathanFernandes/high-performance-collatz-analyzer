#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <algorithm>

namespace collatz {
namespace research {

class FiniteSizeFitAnalyzer {
private:
    struct DataPoint {
        double N;
        double mu;
        double x1; // 1 / ln(N)
        double x2; // 1 / ln(N)^2
    };

    // Helper to solve 3x3 linear system M * beta = V using Cramer's rule
    static bool solve_3x3(const double M[3][3], const double V[3], double beta[3]) {
        double D = M[0][0] * (M[1][1] * M[2][2] - M[1][2] * M[2][1])
                 - M[0][1] * (M[1][0] * M[2][2] - M[1][2] * M[2][0])
                 + M[0][2] * (M[1][0] * M[2][1] - M[1][1] * M[2][0]);

        if (std::abs(D) < 1e-12) return false;

        double Da = V[0] * (M[1][1] * M[2][2] - M[1][2] * M[2][1])
                  - M[0][1] * (V[1] * M[2][2] - M[1][2] * V[2])
                  + M[0][2] * (V[1] * M[2][1] - M[1][1] * V[2]);

        double Db = M[0][0] * (V[1] * M[2][2] - M[1][2] * V[2])
                  - V[0] * (M[1][0] * M[2][2] - M[1][2] * M[2][0])
                  + M[0][2] * (M[1][0] * V[2] - V[1] * M[2][0]);

        double Dc = M[0][0] * (M[1][1] * V[2] - V[1] * M[2][1])
                  - M[0][1] * (M[1][0] * V[2] - V[1] * M[2][0])
                  + V[0] * (M[1][0] * M[2][1] - M[1][1] * M[2][0]);

        beta[0] = Da / D;
        beta[1] = Db / D;
        beta[2] = Dc / D;
        return true;
    }

    static bool fit_model(const std::vector<DataPoint>& data, double& a, double& b, double& c) {
        int n = data.size();
        if (n < 3) return false;

        double M[3][3] = {0};
        double V[3] = {0};

        for (const auto& pt : data) {
            M[0][0] += 1;
            M[0][1] += pt.x1;
            M[0][2] += pt.x2;
            
            M[1][1] += pt.x1 * pt.x1;
            M[1][2] += pt.x1 * pt.x2;
            
            M[2][2] += pt.x2 * pt.x2;
            
            V[0] += pt.mu;
            V[1] += pt.x1 * pt.mu;
            V[2] += pt.x2 * pt.mu;
        }

        // Fill symmetric parts
        M[1][0] = M[0][1];
        M[2][0] = M[0][2];
        M[2][1] = M[1][2];

        double beta[3];
        if (!solve_3x3(M, V, beta)) return false;

        a = beta[0];
        b = beta[1];
        c = beta[2];
        return true;
    }

public:
    static void analyze(const std::string& csv_file, double predict_N = 0.0) {
        std::cout << "\n========================================================\n";
        std::cout << "Research Module 22: Finite Size Fit Analyzer\n";
        std::cout << "Model: mu(N) = a + b/log(N) + c/log(N)^2\n";
        std::cout << "========================================================\n";

        std::ifstream file(csv_file);
        if (!file.is_open()) {
            std::cerr << "[ERROR] Could not open file: " << csv_file << "\n";
            return;
        }

        std::vector<DataPoint> data;
        std::string line;
        
        // Skip header
        std::getline(file, line);

        while (std::getline(file, line)) {
            if (line.empty()) continue;
            std::stringstream ss(line);
            std::string limit_str, mu_str;
            if (std::getline(ss, limit_str, ',') && std::getline(ss, mu_str, ',')) {
                try {
                    double n_val = std::stod(limit_str);
                    double mu_val = std::stod(mu_str);
                    if (n_val > 1.0) {
                        double logN = std::log(n_val);
                        data.push_back({n_val, mu_val, 1.0 / logN, 1.0 / (logN * logN)});
                    }
                } catch (...) {
                    // Ignore parsing errors for individual lines
                }
            }
        }

        int n_pts = data.size();
        if (n_pts < 3) {
            std::cerr << "[ERROR] Not enough valid data points found in CSV (found " << n_pts << ").\n";
            return;
        }

        // 1. Fit full model
        double a = 0, b = 0, c = 0;
        if (!fit_model(data, a, b, c)) {
            std::cerr << "[ERROR] Matrix inversion failed during full model fitting.\n";
            return;
        }

        // Calculate full model R^2
        double mean_mu = 0;
        for (const auto& pt : data) mean_mu += pt.mu;
        mean_mu /= n_pts;

        double ss_tot = 0, ss_res = 0;
        for (const auto& pt : data) {
            double pred = a + b * pt.x1 + c * pt.x2;
            ss_res += (pt.mu - pred) * (pt.mu - pred);
            ss_tot += (pt.mu - mean_mu) * (pt.mu - mean_mu);
        }
        double r_squared = 1.0 - (ss_res / ss_tot);

        // 2. LOOCV for Out-of-Sample metrics
        std::vector<double> oos_predictions;
        std::vector<double> oos_residuals;
        double mae = 0;
        double max_res = 0;

        for (int i = 0; i < n_pts; ++i) {
            std::vector<DataPoint> subset;
            for (int j = 0; j < n_pts; ++j) {
                if (i != j) subset.push_back(data[j]);
            }

            double a_loo, b_loo, c_loo;
            if (fit_model(subset, a_loo, b_loo, c_loo)) {
                double pred = a_loo + b_loo * data[i].x1 + c_loo * data[i].x2;
                double res = data[i].mu - pred;
                double abs_res = std::abs(res);
                
                oos_predictions.push_back(pred);
                oos_residuals.push_back(res);
                mae += abs_res;
                if (abs_res > max_res) max_res = abs_res;
            } else {
                oos_predictions.push_back(0);
                oos_residuals.push_back(0);
            }
        }
        mae /= n_pts;

        // 3. Print Results
        std::cout << "\n--- Full Model Fit ---\n";
        std::cout << std::fixed << std::setprecision(6);
        std::cout << "a (Asymptote) : " << a << "\n";
        std::cout << "b             : " << b << "\n";
        std::cout << "c             : " << c << "\n";
        std::cout << "R^2           : " << r_squared << "\n";

        std::cout << "\n--- Out-of-Sample (LOOCV) Metrics ---\n";
        std::cout << std::scientific << std::setprecision(6);
        std::cout << "Mean Abs Error: " << mae << "\n";
        std::cout << "Max Residual  : " << max_res << "\n";

        std::cout << "\n--- Out-of-Sample Prediction Table ---\n";
        std::cout << std::left << std::setw(15) << "Limit (N)" 
                  << std::setw(15) << "Actual mu" 
                  << std::setw(15) << "Predicted mu" 
                  << "Residual\n";
        std::cout << "--------------------------------------------------------\n";
        
        for (int i = 0; i < n_pts; ++i) {
            std::cout << std::left << std::setw(15) << std::fixed << std::setprecision(0) << data[i].N
                      << std::setw(15) << std::fixed << std::setprecision(8) << data[i].mu
                      << std::setw(15) << std::fixed << std::setprecision(8) << oos_predictions[i]
                      << std::scientific << std::setprecision(6) << oos_residuals[i] << "\n";
        }
        std::cout << "========================================================\n";

        if (predict_N > 1.0) {
            double logN = std::log(predict_N);
            double correction = b / logN + c / (logN * logN);
            double predicted_mu = a + correction;
            double distance = std::abs(predicted_mu - a);

            std::cout << "\n--- Extrapolation ---\n";
            std::cout << std::left << std::setw(18) << "N" << ": " << std::fixed << std::setprecision(0) << predict_N << "\n";
            std::cout << std::setw(18) << "Predicted mu(N)" << ": " << std::fixed << std::setprecision(8) << predicted_mu << "\n";
            std::cout << std::setw(18) << "Correction term" << ": " << std::scientific << std::setprecision(8) << correction << "\n";
            std::cout << std::setw(18) << "Asymptotic limit" << ": " << std::fixed << std::setprecision(8) << a << "\n";
            std::cout << std::setw(18) << "Distance to limit" << ": " << std::scientific << std::setprecision(8) << distance << "\n";
            std::cout << "========================================================\n";
        }
        std::cout << "\n";
    }
};

} // namespace research
} // namespace collatz
