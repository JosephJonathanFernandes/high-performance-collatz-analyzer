#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <random>
#include <set>

namespace collatz {
namespace research {

class BootstrapConfidenceEngine {
private:
    struct DataPoint {
        double N;
        double mu;
    };

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

    static bool fit_model(const std::vector<DataPoint>& data, double& a, double& b, double& c) {
        double M[3][3] = {0};
        double V[3] = {0};
        for (const auto& pt : data) {
            double lN = std::log(pt.N);
            double x1 = 1.0 / lN;
            double x2 = 1.0 / (lN * lN);
            
            M[0][0] += 1;
            M[0][1] += x1;
            M[0][2] += x2;
            M[1][1] += x1 * x1;
            M[1][2] += x1 * x2;
            M[2][2] += x2 * x2;
            V[0] += pt.mu;
            V[1] += x1 * pt.mu;
            V[2] += x2 * pt.mu;
        }
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
    static void analyze(const std::string& csv_file) {
        std::cout << "\n========================================================\n";
        std::cout << "Research Module 33: Bootstrap Confidence Engine\n";
        std::cout << "========================================================\n\n";

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
                        data.push_back({N, mu_obs});
                    }
                } catch (...) {}
            }
        }

        int n = data.size();
        if (n < 4) {
            std::cerr << "[ERROR] Not enough data points.\n";
            return;
        }

        double base_a, base_b, base_c;
        if (!fit_model(data, base_a, base_b, base_c)) {
            std::cerr << "[ERROR] Baseline fit failed.\n";
            return;
        }
        double base_p50 = base_a + base_b / std::log(50000000000.0) + base_c / std::pow(std::log(50000000000.0), 2);
        double base_p100 = base_a + base_b / std::log(100000000000.0) + base_c / std::pow(std::log(100000000000.0), 2);

        int B = 100000;
        std::cout << "Running Non-Parametric Bootstrap (B = " << B << ")\n";
        std::cout << "Sampling with replacement from N=" << n << "...\n\n";

        std::mt19937 gen(42); 
        std::uniform_int_distribution<> dis(0, n - 1);

        std::vector<double> a_vals;
        std::vector<double> p50_vals;
        std::vector<double> p100_vals;
        
        a_vals.reserve(B);
        p50_vals.reserve(B);
        p100_vals.reserve(B);

        int valid_iters = 0;

        while (valid_iters < B) {
            std::vector<DataPoint> sample;
            sample.reserve(n);
            std::set<double> unique_N;

            for (int i = 0; i < n; ++i) {
                int idx = dis(gen);
                sample.push_back(data[idx]);
                unique_N.insert(data[idx].N);
            }

            if (unique_N.size() < 3) continue;

            double a_i, b_i, c_i;
            if (fit_model(sample, a_i, b_i, c_i)) {
                a_vals.push_back(a_i);
                p50_vals.push_back(a_i + b_i / std::log(50000000000.0) + c_i / std::pow(std::log(50000000000.0), 2));
                p100_vals.push_back(a_i + b_i / std::log(100000000000.0) + c_i / std::pow(std::log(100000000000.0), 2));
                valid_iters++;
            }
        }

        std::sort(a_vals.begin(), a_vals.end());
        std::sort(p50_vals.begin(), p50_vals.end());
        std::sort(p100_vals.begin(), p100_vals.end());

        auto get_stats = [](const std::vector<double>& vec, double& mean, double& median, double& stddev, double& ci_lower, double& ci_upper) {
            int size = vec.size();
            double sum = 0;
            for (double v : vec) sum += v;
            mean = sum / size;
            
            double var = 0;
            for (double v : vec) var += (v - mean) * (v - mean);
            stddev = std::sqrt(var / (size - 1));
            
            if (size % 2 == 0) median = (vec[size / 2 - 1] + vec[size / 2]) / 2.0;
            else median = vec[size / 2];

            int idx_lower = static_cast<int>(size * 0.025);
            int idx_upper = static_cast<int>(size * 0.975);
            ci_lower = vec[idx_lower];
            ci_upper = vec[idx_upper];
        };

        double a_mean, a_median, a_std, a_lower, a_upper;
        get_stats(a_vals, a_mean, a_median, a_std, a_lower, a_upper);

        double p50_mean, p50_median, p50_std, p50_lower, p50_upper;
        get_stats(p50_vals, p50_mean, p50_median, p50_std, p50_lower, p50_upper);

        double p100_mean, p100_median, p100_std, p100_lower, p100_upper;
        get_stats(p100_vals, p100_mean, p100_median, p100_std, p100_lower, p100_upper);

        std::cout << "--- 95% Empirical Confidence Intervals ---\n\n";

        std::cout << "Parameter a (Asymptote):\n";
        std::cout << "  Baseline Fit : " << std::fixed << std::setprecision(6) << base_a << "\n";
        std::cout << "  Boot Mean    : " << std::fixed << std::setprecision(6) << a_mean << "\n";
        std::cout << "  Boot Median  : " << std::fixed << std::setprecision(6) << a_median << "\n";
        std::cout << "  Boot StdDev  : " << std::scientific << std::setprecision(4) << a_std << "\n";
        std::cout << "  95% CI       : [" << std::fixed << std::setprecision(6) << a_lower << ", " << a_upper << "]\n\n";

        std::cout << "Prediction (50 Billion):\n";
        std::cout << "  Baseline Fit : " << std::fixed << std::setprecision(6) << base_p50 << "\n";
        std::cout << "  Boot Mean    : " << std::fixed << std::setprecision(6) << p50_mean << "\n";
        std::cout << "  Boot Median  : " << std::fixed << std::setprecision(6) << p50_median << "\n";
        std::cout << "  Boot StdDev  : " << std::scientific << std::setprecision(4) << p50_std << "\n";
        std::cout << "  95% CI       : [" << std::fixed << std::setprecision(6) << p50_lower << ", " << p50_upper << "]\n\n";

        std::cout << "Prediction (100 Billion):\n";
        std::cout << "  Baseline Fit : " << std::fixed << std::setprecision(6) << base_p100 << "\n";
        std::cout << "  Boot Mean    : " << std::fixed << std::setprecision(6) << p100_mean << "\n";
        std::cout << "  Boot Median  : " << std::fixed << std::setprecision(6) << p100_median << "\n";
        std::cout << "  Boot StdDev  : " << std::scientific << std::setprecision(4) << p100_std << "\n";
        std::cout << "  95% CI       : [" << std::fixed << std::setprecision(6) << p100_lower << ", " << p100_upper << "]\n";

        std::cout << "========================================================\n\n";
    }
};

} // namespace research
} // namespace collatz
