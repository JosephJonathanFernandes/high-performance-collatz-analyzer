#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "DataExporter.hpp"

namespace collatz {
namespace research {

class GrowthConvergenceAnalyzer {
public:
    static void analyze(const std::string& csv_file, int start_depth = 30) {
        std::cout << "\n========================================================\n";
        std::cout << "Research Module 17: Growth Constant Convergence\n";
        std::cout << "File: " << csv_file << "\n";
        std::cout << "Analyzing k(depth) as an expanding window from depth " << start_depth << "\n";
        std::cout << "========================================================\n";

        std::ifstream file(csv_file);
        if (!file.is_open()) {
            std::cerr << "[ERROR] Could not open file: " << csv_file << "\n";
            std::cerr << "Make sure you generated the tree data first!\n";
            return;
        }

        std::vector<double> all_X;
        std::vector<double> all_Y;
        std::string line;
        
        // Skip header
        std::getline(file, line);

        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string depth_str, nodes_str;
            if (std::getline(ss, depth_str, ',') && std::getline(ss, nodes_str, ',')) {
                int d = std::stoi(depth_str);
                double nodes = std::stod(nodes_str);
                
                if (d >= start_depth && nodes > 0) {
                    all_X.push_back(d);
                    all_Y.push_back(std::log(nodes));
                }
            }
        }

        if (all_X.size() < 10) {
            std::cerr << "[ERROR] Not enough data points to compute convergence (need at least 10).\n";
            return;
        }

        std::stringstream csv;
        csv << "end_depth,k_value,r_squared,rmse\n";

        std::cout << std::left << std::setw(12) << "End Depth" 
                  << std::setw(14) << "k(depth)" 
                  << std::setw(14) << "R²" 
                  << "RMSE\n";
        std::cout << std::string(50, '-') << "\n";

        // Calculate expanding window from start_depth to d
        double final_k = 0;
        
        for (size_t end_idx = 9; end_idx < all_X.size(); ++end_idx) {
            int n = end_idx + 1;
            double sum_X = 0, sum_Y = 0;
            for (int i = 0; i < n; ++i) {
                sum_X += all_X[i];
                sum_Y += all_Y[i];
            }

            double mean_X = sum_X / n;
            double mean_Y = sum_Y / n;

            double num = 0, den = 0;
            for (int i = 0; i < n; ++i) {
                num += (all_X[i] - mean_X) * (all_Y[i] - mean_Y);
                den += (all_X[i] - mean_X) * (all_X[i] - mean_X);
            }

            double slope = num / den;
            double intercept = mean_Y - slope * mean_X;

            double ss_tot = 0, ss_res = 0, rmse = 0;
            for (int i = 0; i < n; ++i) {
                double y_pred = slope * all_X[i] + intercept;
                ss_res += (all_Y[i] - y_pred) * (all_Y[i] - y_pred);
                ss_tot += (all_Y[i] - mean_Y) * (all_Y[i] - mean_Y);
                rmse += (all_Y[i] - y_pred) * (all_Y[i] - y_pred);
            }

            double r_squared = 1.0 - (ss_res / ss_tot);
            rmse = std::sqrt(rmse / n);
            double k = std::exp(slope);

            int current_depth = (int)all_X[end_idx];
            
            // Print every 5 depths or the very last one
            if (current_depth % 5 == 0 || end_idx == all_X.size() - 1) {
                std::cout << std::left << std::setw(12) << current_depth
                          << std::setw(14) << std::fixed << std::setprecision(6) << k
                          << std::setw(14) << std::setprecision(6) << r_squared
                          << std::setprecision(6) << rmse << "\n";
            }

            csv << current_depth << "," << std::fixed << std::setprecision(6) << k 
                << "," << r_squared << "," << rmse << "\n";
                
            final_k = k;
        }

        std::cout << std::string(50, '-') << "\n";
        std::cout << "Terminal k(depth) limit : " << std::fixed << std::setprecision(6) << final_k << "\n";

        DataExporter::export_csv("growth_convergence.csv", csv.str());
    }
};

} // namespace research
} // namespace collatz
