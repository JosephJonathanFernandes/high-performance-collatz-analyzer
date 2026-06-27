#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>

namespace collatz {
namespace research {

class InfluenceAnalyzer {
private:
    struct DataPoint {
        double N;
        double lN;
        double mu;
    };

    static bool invert_3x3(const double M[3][3], double Minv[3][3]) {
        double det = M[0][0] * (M[1][1] * M[2][2] - M[1][2] * M[2][1])
                   - M[0][1] * (M[1][0] * M[2][2] - M[1][2] * M[2][0])
                   + M[0][2] * (M[1][0] * M[2][1] - M[1][1] * M[2][0]);

        if (std::abs(det) < 1e-15) return false;

        double invdet = 1.0 / det;

        Minv[0][0] = (M[1][1] * M[2][2] - M[2][1] * M[1][2]) * invdet;
        Minv[0][1] = (M[0][2] * M[2][1] - M[0][1] * M[2][2]) * invdet;
        Minv[0][2] = (M[0][1] * M[1][2] - M[0][2] * M[1][1]) * invdet;
        Minv[1][0] = (M[1][2] * M[2][0] - M[1][0] * M[2][2]) * invdet;
        Minv[1][1] = (M[0][0] * M[2][2] - M[0][2] * M[2][0]) * invdet;
        Minv[1][2] = (M[1][0] * M[0][2] - M[0][0] * M[1][2]) * invdet;
        Minv[2][0] = (M[1][0] * M[2][1] - M[2][0] * M[1][1]) * invdet;
        Minv[2][1] = (M[2][0] * M[0][1] - M[0][0] * M[2][1]) * invdet;
        Minv[2][2] = (M[0][0] * M[1][1] - M[1][0] * M[0][1]) * invdet;

        return true;
    }

public:
    static void analyze(const std::string& csv_file) {
        std::cout << "\n========================================================\n";
        std::cout << "Research Module 29: Influence Analyzer (Cook's D)\n";
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
                        data.push_back({N, std::log(N), mu_obs});
                    }
                } catch (...) {}
            }
        }

        int n = data.size();
        if (n < 4) {
            std::cerr << "[ERROR] Not enough data points.\n";
            return;
        }

        int p = 3;

        double M[3][3] = {0};
        double V[3] = {0};

        for (const auto& pt : data) {
            double x1 = 1.0 / pt.lN;
            double x2 = 1.0 / (pt.lN * pt.lN);
            
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

        double Minv[3][3];
        if (!invert_3x3(M, Minv)) {
            std::cerr << "[ERROR] Matrix inversion failed (singular design matrix).\n";
            return;
        }

        double beta[3] = {0};
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                beta[i] += Minv[i][j] * V[j];
            }
        }

        std::vector<double> residuals(n, 0);
        double sse = 0;
        for (int i = 0; i < n; ++i) {
            double x1 = 1.0 / data[i].lN;
            double x2 = 1.0 / (data[i].lN * data[i].lN);
            double pred = beta[0] + beta[1] * x1 + beta[2] * x2;
            residuals[i] = data[i].mu - pred;
            sse += residuals[i] * residuals[i];
        }
        double mse = sse / (n - p);

        std::vector<double> leverage(n, 0);
        std::vector<double> cook_d(n, 0);
        
        double leverage_threshold = 2.0 * p / n;
        double cook_threshold = 4.0 / n; 

        for (int i = 0; i < n; ++i) {
            double rowX[3] = {1.0, 1.0 / data[i].lN, 1.0 / (data[i].lN * data[i].lN)};
            double h_i = 0;
            for (int j = 0; j < 3; ++j) {
                double temp = 0;
                for (int k = 0; k < 3; ++k) {
                    temp += Minv[j][k] * rowX[k];
                }
                h_i += rowX[j] * temp;
            }
            leverage[i] = h_i;
            
            double e_i = residuals[i];
            cook_d[i] = (e_i * e_i / (p * mse)) * (h_i / ((1.0 - h_i) * (1.0 - h_i)));
        }

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

        std::cout << "Leverage threshold  : " << std::fixed << std::setprecision(4) << leverage_threshold << " (2p/n)\n";
        std::cout << "Cook's D threshold  : " << std::fixed << std::setprecision(4) << cook_threshold << " (4/n)\n\n";

        std::cout << "--------------------------------------------------------------------------------\n";
        std::cout << std::left << std::setw(15) << "Point" 
                  << std::setw(15) << "Leverage (h)" 
                  << std::setw(20) << "Residual (e)" 
                  << std::setw(15) << "Cook's D" 
                  << "Flags\n";
        std::cout << "--------------------------------------------------------------------------------\n";

        for (int i = 0; i < n; ++i) {
            std::string flags = "";
            if (leverage[i] > leverage_threshold) flags += "[High Lev] ";
            if (cook_d[i] > cook_threshold) flags += "[High Inf] ";

            std::cout << std::left << std::setw(15) << format_N(data[i].N)
                      << std::setw(15) << std::fixed << std::setprecision(6) << leverage[i]
                      << std::showpos << std::setw(20) << std::fixed << std::setprecision(6) << residuals[i] << std::noshowpos
                      << std::setw(15) << std::fixed << std::setprecision(6) << cook_d[i]
                      << flags << "\n";
        }
        std::cout << "--------------------------------------------------------------------------------\n\n";

        std::cout << "========================================================\n\n";
    }
};

} // namespace research
} // namespace collatz
