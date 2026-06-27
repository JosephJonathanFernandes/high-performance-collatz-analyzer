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

class StabilityAnalyzer {
private:
    struct DataPoint {
        double N;
        double mu;
    };

    struct JackknifeResult {
        double dropped_N;
        double a;
        double b;
        double c;
        double delta_a;
        double delta_b;
        double delta_c;
        double pred_50B;
        double pred_100B;
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
        std::cout << "Research Module 28: Stability Analyzer (Jackknife)\n";
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

        double a_full = 0, b_full = 0, c_full = 0;
        if (!fit_model(data, a_full, b_full, c_full)) {
            std::cerr << "[ERROR] Baseline fit failed.\n";
            return;
        }

        std::cout << "Baseline Model (Full Data: " << n << " points):\n";
        std::cout << "a = " << std::fixed << std::setprecision(6) << a_full << "\n";
        std::cout << "b = " << std::fixed << std::setprecision(6) << b_full << "\n";
        std::cout << "c = " << std::fixed << std::setprecision(6) << c_full << "\n\n";

        std::vector<JackknifeResult> results;

        for (int i = 0; i < n; ++i) {
            std::vector<DataPoint> subset;
            for (int j = 0; j < n; ++j) {
                if (i != j) subset.push_back(data[j]);
            }
            
            double a_i, b_i, c_i;
            if (fit_model(subset, a_i, b_i, c_i)) {
                JackknifeResult res;
                res.dropped_N = data[i].N;
                res.a = a_i;
                res.b = b_i;
                res.c = c_i;
                res.delta_a = a_i - a_full;
                res.delta_b = b_i - b_full;
                res.delta_c = c_i - c_full;
                res.pred_50B = a_i + b_i / std::log(50000000000.0) + c_i / std::pow(std::log(50000000000.0), 2);
                res.pred_100B = a_i + b_i / std::log(100000000000.0) + c_i / std::pow(std::log(100000000000.0), 2);
                results.push_back(res);
            }
        }

        int k = results.size();
        
        double a_mean = 0, p50_mean = 0, p100_mean = 0;
        for (const auto& res : results) {
            a_mean += res.a;
            p50_mean += res.pred_50B;
            p100_mean += res.pred_100B;
        }
        a_mean /= k;
        p50_mean /= k;
        p100_mean /= k;

        double a_var = 0, p50_var = 0, p100_var = 0;
        double a_min = results[0].a, a_max = results[0].a;
        double p50_min = results[0].pred_50B, p50_max = results[0].pred_50B;
        double p100_min = results[0].pred_100B, p100_max = results[0].pred_100B;

        for (const auto& res : results) {
            a_var += (res.a - a_mean) * (res.a - a_mean);
            if (res.a < a_min) a_min = res.a;
            if (res.a > a_max) a_max = res.a;
            
            p50_var += (res.pred_50B - p50_mean) * (res.pred_50B - p50_mean);
            if (res.pred_50B < p50_min) p50_min = res.pred_50B;
            if (res.pred_50B > p50_max) p50_max = res.pred_50B;
            
            p100_var += (res.pred_100B - p100_mean) * (res.pred_100B - p100_mean);
            if (res.pred_100B < p100_min) p100_min = res.pred_100B;
            if (res.pred_100B > p100_max) p100_max = res.pred_100B;
        }

        double a_std = std::sqrt(a_var / (k - 1));
        double p50_std = std::sqrt(p50_var / (k - 1));
        double p100_std = std::sqrt(p100_var / (k - 1));

        auto format_N = [](double N) {
            std::ostringstream ss;
            if (N >= 1e9) {
                double val = N / 1e9;
                if (val == std::floor(val)) ss << static_cast<int>(val) << "B";
                else ss << std::fixed << std::setprecision(1) << val << "B";
            } else if (N >= 1e6) {
                double val = N / 1e6;
                if (val == std::floor(val)) ss << static_cast<int>(val) << "M";
                else ss << std::fixed << std::setprecision(1) << val << "M";
            } else {
                ss << static_cast<int>(N);
            }
            return ss.str();
        };

        std::cout << "Jackknife Leave-One-Out Analysis\n";
        std::cout << "-------------------------------------------------------------------------------------------------\n";
        std::cout << std::left << std::setw(15) << "Dropped" 
                  << std::setw(15) << "Delta a" 
                  << std::setw(15) << "Delta b" 
                  << std::setw(15) << "Delta c" 
                  << std::setw(15) << "Pred 50B" 
                  << "Pred 100B\n";
        std::cout << "-------------------------------------------------------------------------------------------------\n";

        for (const auto& res : results) {
            std::cout << std::left << std::setw(15) << format_N(res.dropped_N)
                      << std::showpos 
                      << std::setw(15) << std::fixed << std::setprecision(6) << res.delta_a
                      << std::setw(15) << std::fixed << std::setprecision(6) << res.delta_b
                      << std::setw(15) << std::fixed << std::setprecision(6) << res.delta_c
                      << std::noshowpos
                      << std::setw(15) << std::fixed << std::setprecision(6) << res.pred_50B
                      << std::fixed << std::setprecision(6) << res.pred_100B << "\n";
        }
        std::cout << "-------------------------------------------------------------------------------------------------\n\n";

        std::cout << "--- Parameter Stability ---\n\n";
        std::cout << "a:\n";
        std::cout << "mean  = " << std::fixed << std::setprecision(6) << a_mean << "\n";
        std::cout << "std   = " << std::scientific << std::setprecision(4) << a_std << "\n";
        std::cout << "range = [" << std::fixed << std::setprecision(6) << a_min << " to " << a_max << "]\n\n";

        std::cout << "50B prediction:\n";
        std::cout << "mean  = " << std::fixed << std::setprecision(6) << p50_mean << "\n";
        std::cout << "std   = " << std::scientific << std::setprecision(4) << p50_std << "\n";
        std::cout << "range = [" << std::fixed << std::setprecision(6) << p50_min << " to " << p50_max << "]\n\n";

        std::cout << "100B prediction:\n";
        std::cout << "mean  = " << std::fixed << std::setprecision(6) << p100_mean << "\n";
        std::cout << "std   = " << std::scientific << std::setprecision(4) << p100_std << "\n";
        std::cout << "range = [" << std::fixed << std::setprecision(6) << p100_min << " to " << p100_max << "]\n";

        std::cout << "========================================================\n\n";
    }
};

} // namespace research
} // namespace collatz
