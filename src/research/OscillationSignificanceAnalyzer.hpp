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

namespace collatz {
namespace research {

class OscillationSignificanceAnalyzer {
private:
    struct DataPoint {
        double N;
        double lN;
        double r;
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

    static double find_best_r2(const std::vector<DataPoint>& data) {
        double best_r2 = -1.0;
        int n = data.size();
        
        double mean_r = 0;
        for (const auto& pt : data) mean_r += pt.r;
        mean_r /= n;

        double ss_tot = 0;
        for (const auto& pt : data) {
            ss_tot += (pt.r - mean_r) * (pt.r - mean_r);
        }
        
        if (ss_tot == 0) return 0.0;

        for (double alpha = 1.0; alpha <= 50.0; alpha += 0.1) {
            double M[3][3] = {0};
            double V[3] = {0};
            for (const auto& pt : data) {
                double x1 = std::sin(alpha * pt.lN);
                double x2 = std::cos(alpha * pt.lN);
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
                double ss_res = 0;
                for (const auto& pt : data) {
                    double pred = beta[0] + beta[1] * std::sin(alpha * pt.lN) + beta[2] * std::cos(alpha * pt.lN);
                    ss_res += (pt.r - pred) * (pt.r - pred);
                }
                double r2 = 1.0 - (ss_res / ss_tot);
                if (r2 > best_r2) best_r2 = r2;
            }
        }
        return best_r2;
    }

public:
    static void analyze(const std::string& csv_file) {
        std::cout << "\n========================================================\n";
        std::cout << "Research Module 27: Oscillation Significance Analyzer\n";
        std::cout << "========================================================\n\n";

        const double a_f = -0.291595;
        const double b_f = -0.630446;
        const double c_f = -6.923932;

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
                        data.push_back({N, lN, mu_obs - mu_exp});
                    }
                } catch (...) {}
            }
        }

        if (data.size() < 4) {
            std::cerr << "[ERROR] Not enough data points.\n";
            return;
        }

        std::cout << "Dataset: " << data.size() << " points\n";
        
        std::cout << "Computing true observed R^2 (alpha 1.0 - 50.0)...\n";
        double r2_obs = find_best_r2(data);
        std::cout << "Observed Oscillator R^2: " << std::fixed << std::setprecision(5) << r2_obs << "\n\n";

        const int iterations = 10000;
        std::cout << "Running Monte Carlo Permutation Test (" << iterations << " iterations)...\n";
        
        std::mt19937 rng(42); 
        int count_greater_equal = 0;
        
        std::vector<double> r_values;
        for (const auto& pt : data) r_values.push_back(pt.r);

        for (int i = 0; i < iterations; ++i) {
            std::shuffle(r_values.begin(), r_values.end(), rng);
            
            std::vector<DataPoint> shuffled_data = data;
            for (size_t j = 0; j < data.size(); ++j) {
                shuffled_data[j].r = r_values[j];
            }
            
            double r2_sim = find_best_r2(shuffled_data);
            if (r2_sim >= r2_obs) {
                count_greater_equal++;
            }
            
            if ((i + 1) % 2500 == 0) {
                std::cout << "  ... completed " << (i + 1) << " iterations\n";
            }
        }

        double p_value = static_cast<double>(count_greater_equal) / iterations;

        std::cout << "\n--- Monte Carlo Results ---\n";
        std::cout << "Simulations where R^2_sim >= R^2_obs : " << count_greater_equal << " / " << iterations << "\n";
        std::cout << "Empirical p-value: " << std::fixed << std::setprecision(4) << p_value << "\n\n";

        std::cout << "--- Statistical Interpretation ---\n";
        if (p_value < 0.01) {
            std::cout << "Result: POTENTIAL STRUCTURAL DISCOVERY (p < 0.01)\n";
            std::cout << "The oscillation is highly unlikely to be random noise.\n";
        } else if (p_value < 0.05) {
            std::cout << "Result: WEAK EVIDENCE (0.01 <= p < 0.05)\n";
            std::cout << "The oscillation is unusual but requires more data to confirm.\n";
        } else {
            std::cout << "Result: LOOK-ELSEWHERE ARTIFACT (p >= 0.05)\n";
            std::cout << "The oscillator fit is consistent with random noise swept across many frequencies.\n";
        }

        std::cout << "========================================================\n\n";
    }
};

} // namespace research
} // namespace collatz
