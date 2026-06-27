#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <functional>

namespace collatz {
namespace research {

class ResidualStructureAnalyzer {
private:
    struct DataPoint {
        double N;
        double mu_obs;
        double mu_exp;
        double r;
    };

    static bool solve_2x2(const double M[2][2], const double V[2], double beta[2]) {
        double D = M[0][0] * M[1][1] - M[0][1] * M[1][0];
        if (std::abs(D) < 1e-15) return false;
        beta[0] = (V[0] * M[1][1] - M[0][1] * V[1]) / D;
        beta[1] = (M[0][0] * V[1] - V[0] * M[1][0]) / D;
        return true;
    }

    static bool solve_3x3(const double M[3][3], const double V[3], double beta[3]) {
        double D = M[0][0] * (M[1][1] * M[2][2] - M[1][2] * M[2][1])
                 - M[0][1] * (M[1][0] * M[2][2] - M[1][2] * M[2][0])
                 + M[0][2] * (M[1][0] * M[2][1] - M[1][1] * M[2][0]);

        if (std::abs(D) < 1e-15) return false;

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

    static double calculate_r2(const std::vector<DataPoint>& data, std::function<double(double)> predictor) {
        double ss_tot = 0;
        double ss_res = 0;
        double mean_r = 0;
        for (const auto& pt : data) mean_r += pt.r;
        mean_r /= data.size();

        for (const auto& pt : data) {
            double pred = predictor(pt.N);
            ss_res += (pt.r - pred) * (pt.r - pred);
            ss_tot += (pt.r - mean_r) * (pt.r - mean_r);
        }
        if (ss_tot == 0) return 0.0;
        return 1.0 - (ss_res / ss_tot);
    }

    static void test_linear(const std::string& name, const std::vector<DataPoint>& data, std::function<double(double)> tx) {
        double M[2][2] = {0};
        double V[2] = {0};
        for (const auto& pt : data) {
            double x = tx(pt.N);
            M[0][0] += 1;
            M[0][1] += x;
            M[1][1] += x * x;
            V[0] += pt.r;
            V[1] += x * pt.r;
        }
        M[1][0] = M[0][1];
        
        double beta[2];
        if (solve_2x2(M, V, beta)) {
            double r2 = calculate_r2(data, [&](double N){ return beta[0] + beta[1] * tx(N); });
            std::cout << std::left << std::setw(30) << name 
                      << "R^2: " << std::fixed << std::setprecision(5) << r2 << "\n";
        } else {
            std::cout << std::left << std::setw(30) << name << "Fit failed\n";
        }
    }

public:
    static void analyze(const std::string& csv_file) {
        std::cout << "\n========================================================\n";
        std::cout << "Research Module 26: Residual Structure Analyzer\n";
        std::cout << "========================================================\n\n";

        const double a_f = -0.291595;
        const double b_f = -0.630446;
        const double c_f = -6.923932;

        std::cout << "Frozen Model Parameters:\n";
        std::cout << "a = " << a_f << ", b = " << b_f << ", c = " << c_f << "\n\n";

        std::ifstream file(csv_file);
        if (!file.is_open()) {
            std::cerr << "[ERROR] Could not open file: " << csv_file << "\n";
            return;
        }

        std::vector<DataPoint> data;
        std::string line;
        std::getline(file, line); 

        while (std::getline(file, line)) {
            if (line.empty()) continue;
            std::stringstream ss(line);
            std::string limit_str, mu_str;
            if (std::getline(ss, limit_str, ',') && std::getline(ss, mu_str, ',')) {
                try {
                    double N = std::stod(limit_str);
                    double mu_obs = std::stod(mu_str);
                    if (N > 1.0) {
                        double lN = std::log(N);
                        double mu_exp = a_f + b_f / lN + c_f / (lN * lN);
                        data.push_back({N, mu_obs, mu_exp, mu_obs - mu_exp});
                    }
                } catch (...) {}
            }
        }

        int n = data.size();
        if (n < 4) {
            std::cerr << "[ERROR] Not enough data points.\n";
            return;
        }

        std::cout << "Dataset: " << n << " points\n";
        double max_r = 0;
        double sum_abs_r = 0;
        for (const auto& pt : data) {
            if (std::abs(pt.r) > max_r) max_r = std::abs(pt.r);
            sum_abs_r += std::abs(pt.r);
        }
        std::cout << "Max Residual : " << std::scientific << std::setprecision(4) << max_r << "\n";
        std::cout << "Mean Abs Res : " << std::scientific << std::setprecision(4) << (sum_abs_r / n) << "\n\n";

        std::cout << "--- Structural Regression Tests ---\n";
        test_linear("1/log^3(N)", data, [](double N) { return 1.0 / std::pow(std::log(N), 3); });
        test_linear("1/N", data, [](double N) { return 1.0 / N; });
        
        std::cout << "\n--- Non-parametric Tests ---\n";
        
        double mean_r = 0;
        for (const auto& pt : data) mean_r += pt.r;
        mean_r /= n;

        double num = 0, den = 0;
        for (int i = 0; i < n - 1; ++i) {
            num += (data[i].r - mean_r) * (data[i+1].r - mean_r);
        }
        for (int i = 0; i < n; ++i) {
            den += (data[i].r - mean_r) * (data[i].r - mean_r);
        }
        double ac1 = (den == 0) ? 0 : num / den;
        std::cout << "Lag-1 Autocorrelation: " << std::fixed << std::setprecision(4) << ac1 << "\n";

        int n_pos = 0, n_neg = 0, runs = 1;
        bool last_pos = (data[0].r > 0);
        for (int i = 0; i < n; ++i) {
            bool pos = (data[i].r > 0);
            if (pos) n_pos++; else n_neg++;
            if (i > 0 && pos != last_pos) {
                runs++;
                last_pos = pos;
            }
        }
        
        double expected_runs = 1.0 + (2.0 * n_pos * n_neg) / n;
        double var_runs = (2.0 * n_pos * n_neg * (2.0 * n_pos * n_neg - n)) / (n * n * (n - 1.0));
        double z_score = (var_runs > 0) ? (runs - expected_runs) / std::sqrt(var_runs) : 0;
        
        std::cout << "Sign Runs: " << runs << " (Expected: " << std::fixed << std::setprecision(2) << expected_runs << ")\n";
        std::cout << "Runs Z-Score: " << std::fixed << std::setprecision(3) << z_score << "\n";

        std::cout << "\n--- Log-Periodic Sweep (1.0 to 50.0) ---\n";
        
        double best_alpha = 0, best_r2 = -1.0;
        double best_d = 0, best_beta = 0;

        for (double alpha = 1.0; alpha <= 50.0; alpha += 0.1) {
            double M[3][3] = {0};
            double V[3] = {0};
            for (const auto& pt : data) {
                double lN = std::log(pt.N);
                double x1 = std::sin(alpha * lN);
                double x2 = std::cos(alpha * lN);
                M[0][0] += 1;
                M[0][1] += x1;
                M[0][2] += x2;
                M[1][1] += x1 * x1;
                M[1][2] += x1 * x2;
                M[2][2] += x2 * x2;
                V[0] += pt.r;
                V[1] += x1 * pt.r;
                V[2] += x2 * pt.r;
            }
            M[1][0] = M[0][1];
            M[2][0] = M[0][2];
            M[2][1] = M[1][2];

            double beta[3];
            if (solve_3x3(M, V, beta)) {
                double r2 = calculate_r2(data, [&](double N){ 
                    double lN = std::log(N);
                    return beta[0] + beta[1] * std::sin(alpha * lN) + beta[2] * std::cos(alpha * lN);
                });
                
                if (r2 > best_r2) {
                    best_r2 = r2;
                    best_alpha = alpha;
                    best_d = std::sqrt(beta[1]*beta[1] + beta[2]*beta[2]);
                    best_beta = std::atan2(beta[2], beta[1]);
                }
            }
        }

        std::cout << "Best Oscillatory Fit:\n";
        std::cout << "r(N) = d * sin(alpha * log(N) + beta)\n";
        std::cout << "Alpha (Freq) : " << std::fixed << std::setprecision(4) << best_alpha << "\n";
        std::cout << "d (Amplitude): " << std::scientific << std::setprecision(4) << best_d << "\n";
        std::cout << "Beta (Phase) : " << std::fixed << std::setprecision(4) << best_beta << "\n";
        std::cout << "Oscillator R2: " << std::fixed << std::setprecision(5) << best_r2 << "\n";

        std::cout << "========================================================\n\n";
    }
};

} // namespace research
} // namespace collatz
