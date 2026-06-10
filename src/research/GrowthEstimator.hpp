#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>
#include <iomanip>

namespace collatz {
namespace research {

class GrowthEstimator {
public:
    static void analyze(const std::string& csv_file, int start_depth, int end_depth) {
        std::cout << "\n========================================================\n";
        std::cout << "Research Module 7: Growth Constant Estimator\n";
        std::cout << "Fitting log(N(d)) vs d via Least Squares [" << start_depth << " to " << end_depth << "]\n";
        std::cout << "========================================================\n";

        std::ifstream file(csv_file);
        if (!file.is_open()) {
            std::cerr << "[ERROR] Could not open file: " << csv_file << "\n";
            std::cerr << "Make sure you generated the tree data first!\n";
            return;
        }

        std::vector<double> X;
        std::vector<double> Y;
        std::string line;
        
        // Skip header
        std::getline(file, line);

        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string depth_str, nodes_str;
            if (std::getline(ss, depth_str, ',') && std::getline(ss, nodes_str, ',')) {
                int d = std::stoi(depth_str);
                double nodes = std::stod(nodes_str);
                
                if (d >= start_depth && d <= end_depth && nodes > 0) {
                    X.push_back(d);
                    Y.push_back(std::log(nodes));
                }
            }
        }

        if (X.size() < 2) {
            std::cerr << "[ERROR] Not enough data points in range.\n";
            return;
        }

        int n = X.size();
        double sum_X = 0, sum_Y = 0;
        for (int i = 0; i < n; ++i) {
            sum_X += X[i];
            sum_Y += Y[i];
        }

        double mean_X = sum_X / n;
        double mean_Y = sum_Y / n;

        double num = 0, den = 0;
        for (int i = 0; i < n; ++i) {
            num += (X[i] - mean_X) * (Y[i] - mean_Y);
            den += (X[i] - mean_X) * (X[i] - mean_X);
        }

        double slope = num / den;
        double intercept = mean_Y - slope * mean_X;

        double ss_tot = 0, ss_res = 0;
        double rmse = 0;
        for (int i = 0; i < n; ++i) {
            double y_pred = slope * X[i] + intercept;
            ss_res += (Y[i] - y_pred) * (Y[i] - y_pred);
            ss_tot += (Y[i] - mean_Y) * (Y[i] - mean_Y);
            rmse += (Y[i] - y_pred) * (Y[i] - y_pred);
        }

        double r_squared = 1.0 - (ss_res / ss_tot);
        rmse = std::sqrt(rmse / n);

        // Standard Error of Slope for Confidence Interval
        double se_slope = std::sqrt(ss_res / ((n - 2) * den));
        // Approximate 95% CI multiplier
        double ci_95 = 1.96 * se_slope;
        
        double growth_constant = std::exp(slope);
        double ci_lower = std::exp(slope - ci_95);
        double ci_upper = std::exp(slope + ci_95);

        std::cout << "Data Points Fit      : " << n << "\n";
        std::cout << "Log-Space Slope (m)  : " << std::fixed << std::setprecision(6) << slope << " +/- " << ci_95 << " (95% CI)\n";
        std::cout << "Intercept (b)        : " << intercept << "\n";
        std::cout << "--------------------------------------------------------\n";
        std::cout << "Growth Constant (k)  : " << growth_constant << "\n";
        std::cout << "95% Conf. Interval   : [" << ci_lower << ", " << ci_upper << "]\n";
        std::cout << "R-squared (R^2)      : " << r_squared << "\n";
        std::cout << "RMSE (Log Error)     : " << rmse << "\n";
        std::cout << "========================================================\n";
    }
};

} // namespace research
} // namespace collatz
