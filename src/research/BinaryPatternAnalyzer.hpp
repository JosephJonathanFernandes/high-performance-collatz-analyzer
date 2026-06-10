#pragma once

#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <sstream>
#include "DataExporter.hpp"

namespace collatz {
namespace research {

class BinaryPatternAnalyzer {
public:
    static double pearson_correlation(const std::vector<double>& X, const std::vector<double>& Y) {
        if (X.empty() || X.size() != Y.size()) return 0.0;
        int n = X.size();
        double sum_X = 0, sum_Y = 0;
        for (int i = 0; i < n; ++i) { sum_X += X[i]; sum_Y += Y[i]; }
        double mean_X = sum_X / n;
        double mean_Y = sum_Y / n;
        double num = 0, den_X = 0, den_Y = 0;
        for (int i = 0; i < n; ++i) {
            num += (X[i] - mean_X) * (Y[i] - mean_Y);
            den_X += (X[i] - mean_X) * (X[i] - mean_X);
            den_Y += (Y[i] - mean_Y) * (Y[i] - mean_Y);
        }
        if (den_X == 0 || den_Y == 0) return 0.0;
        return num / std::sqrt(den_X * den_Y);
    }

    static std::vector<double> solve_system(std::vector<std::vector<double>> A, std::vector<double> B) {
        int n = B.size();
        for (int i = 0; i < n; i++) {
            double maxEl = std::abs(A[i][i]);
            int maxRow = i;
            for (int k = i + 1; k < n; k++) {
                if (std::abs(A[k][i]) > maxEl) {
                    maxEl = std::abs(A[k][i]);
                    maxRow = k;
                }
            }
            for (int k = i; k < n; k++) std::swap(A[maxRow][k], A[i][k]);
            std::swap(B[maxRow], B[i]);
            
            for (int k = i + 1; k < n; k++) {
                double c = -A[k][i] / A[i][i];
                for (int j = i; j < n; j++) {
                    if (i == j) A[k][j] = 0;
                    else A[k][j] += c * A[i][j];
                }
                B[k] += c * B[i];
            }
        }
        
        std::vector<double> x(n);
        for (int i = n - 1; i >= 0; i--) {
            x[i] = B[i];
            for (int j = i + 1; j < n; j++) x[i] -= A[i][j] * x[j];
            x[i] = x[i] / A[i][i];
        }
        return x;
    }

    static void multiple_regression(const std::vector<double>& Y, const std::vector<std::vector<double>>& X_cols, const std::vector<std::string>& feature_names) {
        int n = Y.size();
        int k = X_cols.size(); 
        int p = k + 1; 
        
        std::vector<std::vector<double>> X(n, std::vector<double>(p, 1.0));
        for(int i=0; i<n; ++i) {
            for(int j=0; j<k; ++j) X[i][j+1] = X_cols[j][i];
        }
        
        std::vector<std::vector<double>> XtX(p, std::vector<double>(p, 0.0));
        std::vector<double> XtY(p, 0.0);
        
        for(int i=0; i<p; ++i) {
            for(int j=0; j<p; ++j) {
                for(int r=0; r<n; ++r) XtX[i][j] += X[r][i] * X[r][j];
            }
            for(int r=0; r<n; ++r) XtY[i] += X[r][i] * Y[r];
        }
        
        std::vector<double> weights = solve_system(XtX, XtY);
        
        double sum_Y = 0;
        for(double y : Y) sum_Y += y;
        double mean_Y = sum_Y / n;
        
        double ss_tot = 0, ss_res = 0;
        for(int i=0; i<n; ++i) {
            double y_pred = 0;
            for(int j=0; j<p; ++j) y_pred += weights[j] * X[i][j];
            ss_tot += (Y[i] - mean_Y) * (Y[i] - mean_Y);
            ss_res += (Y[i] - y_pred) * (Y[i] - y_pred);
        }
        
        double r_squared = 1.0 - (ss_res / ss_tot);
        
        std::cout << "\n--- Multiple Linear Regression Model ---\n";
        std::cout << "Predicting: avg_steps from " << k << " features\n";
        std::cout << "Joint R^2 Score   : " << std::fixed << std::setprecision(4) << r_squared << "\n";
        std::cout << "Intercept         : " << weights[0] << "\n";
        std::cout << "Weights:\n";
        for (int j = 0; j < k; ++j) {
            std::cout << "  " << std::left << std::setw(25) << feature_names[j] << ": " << weights[j+1] << "\n";
        }
        std::cout << "----------------------------------------\n";
    }

    static void analyze(unsigned long long limit, unsigned long long modulo) {
        // modulo must be a power of 2 for clean binary representation
        if ((modulo & (modulo - 1)) != 0) {
            std::cerr << "[ERROR] Modulo must be a power of 2 for Binary Pattern Analysis.\n";
            return;
        }

        int bit_width = 0;
        unsigned long long temp = modulo;
        while (temp > 1) {
            temp >>= 1;
            bit_width++;
        }

        struct ClassStats {
            unsigned long long count = 0;
            unsigned long long total_steps = 0;
            unsigned long long total_v2 = 0;
            unsigned long long total_peak = 0;
            double total_odd_multiplier = 0.0;
            
            // Binary features
            int max_run_1s = 0;
            int max_run_0s = 0;
            int hamming_weight = 0;
            int alternation_score = 0;
            
            // Advanced features
            int trailing_ones = 0;
            int trailing_zeros_3n_plus_1 = 0;
            int mod_3 = 0;
            int mod_9 = 0;
            int mod_27 = 0;
            double bit_entropy = 0.0;
        };

        std::vector<ClassStats> stats(modulo);

        // Compute binary metrics for each residue class
        for (unsigned long long r = 0; r < modulo; ++r) {
            int current_run_1 = 0, max_run_1 = 0;
            int current_run_0 = 0, max_run_0 = 0;
            int weight = 0;
            int alternations = 0;
            int last_bit = -1;
            int trailing_1s = 0;
            bool still_trailing = true;

            for (int i = 0; i < bit_width; ++i) {
                int bit = (r >> i) & 1;
                if (bit == 1) {
                    weight++;
                    current_run_1++;
                    if (current_run_1 > max_run_1) max_run_1 = current_run_1;
                    current_run_0 = 0;
                    if (still_trailing) trailing_1s++;
                } else {
                    current_run_0++;
                    if (current_run_0 > max_run_0) max_run_0 = current_run_0;
                    current_run_1 = 0;
                    still_trailing = false;
                }
                
                if (last_bit != -1 && bit != last_bit) {
                    alternations++;
                }
                last_bit = bit;
            }

            stats[r].max_run_1s = max_run_1;
            stats[r].max_run_0s = max_run_0;
            stats[r].hamming_weight = weight;
            stats[r].alternation_score = alternations;
            stats[r].trailing_ones = trailing_1s;

            unsigned long long r_next = 3 * r + 1;
            int tz = 0;
            while ((r_next & 1) == 0 && r_next > 0) {
                tz++;
                r_next >>= 1;
            }
            stats[r].trailing_zeros_3n_plus_1 = tz;

            stats[r].mod_3 = r % 3;
            stats[r].mod_9 = r % 9;
            stats[r].mod_27 = r % 27;

            double p1 = (double)weight / bit_width;
            double p0 = 1.0 - p1;
            double entropy = 0.0;
            if (p1 > 0) entropy -= p1 * std::log2(p1);
            if (p0 > 0) entropy -= p0 * std::log2(p0);
            stats[r].bit_entropy = entropy;
        }

        std::cout << "\n========================================================\n";
        std::cout << "Research Module 8: Binary Pattern Analyzer\n";
        std::cout << "Analyzing modulo: " << modulo << " (" << bit_width << " bits) up to limit: " << limit << "\n";
        std::cout << "========================================================\n";

        auto start_time = std::chrono::high_resolution_clock::now();

        // Standard simulation identical to ResidueAnalyzer
        for (unsigned long long i = 1; i <= limit; i+=2) {
            unsigned long long current = i;
            unsigned long long peak = i;
            int steps = 0;
            int first_v2 = 0;
            double total_mult = 0.0;
            int mult_count = 0;

            unsigned long long next_n = 3 * current + 1;
            if (next_n > peak) peak = next_n;
            int current_v2 = 0;
            while ((next_n & 1) == 0) {
                next_n >>= 1;
                first_v2++;
                current_v2++;
                steps++;
            }
            if (current_v2 > 0) {
                total_mult += (3.0 / (1ULL << current_v2));
                mult_count++;
            }
            
            current = next_n;
            while (current != 1) {
                if (current > peak) peak = current;
                if (current & 1) {
                    current = 3 * current + 1;
                    int v2 = 0;
                    while ((current & 1) == 0) {
                        current >>= 1;
                        steps++;
                        v2++;
                    }
                    total_mult += (3.0 / (1ULL << v2));
                    mult_count++;
                } else {
                    current >>= 1;
                    steps++;
                }
            }

            unsigned long long r = i % modulo;
            stats[r].count++;
            stats[r].total_steps += steps;
            stats[r].total_v2 += first_v2;
            stats[r].total_peak += peak;
            if (mult_count > 0) {
                stats[r].total_odd_multiplier += (total_mult / mult_count);
            }
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

        std::stringstream csv;
        csv << "residue,max_run_1s,max_run_0s,hamming_weight,alternations,trailing_ones,tz_3n1,mod3,mod9,mod27,entropy,avg_odd_mult,count,avg_stopping_time,avg_v2,avg_peak\n";

        std::vector<double> avg_steps_list, avg_peak_list, run_1s_list, run_0s_list;
        std::vector<double> hamming_list, alt_score_list, avg_v2_list;
        std::vector<double> trailing_ones_list, tz_3n1_list, mod3_list, mod9_list;
        std::vector<double> mod27_list, entropy_list, avg_odd_mult_list;

        for (unsigned long long r = 0; r < modulo; ++r) {
            if (stats[r].count == 0) continue;
            double avg_steps = (double)stats[r].total_steps / stats[r].count;
            double avg_v2 = (double)stats[r].total_v2 / stats[r].count;
            double avg_peak = (double)stats[r].total_peak / stats[r].count;
            double avg_odd_mult = stats[r].total_odd_multiplier / stats[r].count;
            
            csv << r << "," << stats[r].max_run_1s << "," << stats[r].max_run_0s << "," 
                << stats[r].hamming_weight << "," << stats[r].alternation_score << ","
                << stats[r].trailing_ones << "," << stats[r].trailing_zeros_3n_plus_1 << ","
                << stats[r].mod_3 << "," << stats[r].mod_9 << "," << stats[r].mod_27 << ","
                << stats[r].bit_entropy << "," << avg_odd_mult << ","
                << stats[r].count << "," << avg_steps << "," << avg_v2 << "," << avg_peak << "\n";

            avg_steps_list.push_back(avg_steps);
            avg_peak_list.push_back(avg_peak);
            avg_v2_list.push_back(avg_v2);
            run_1s_list.push_back(stats[r].max_run_1s);
            run_0s_list.push_back(stats[r].max_run_0s);
            hamming_list.push_back(stats[r].hamming_weight);
            alt_score_list.push_back(stats[r].alternation_score);
            trailing_ones_list.push_back(stats[r].trailing_ones);
            tz_3n1_list.push_back(stats[r].trailing_zeros_3n_plus_1);
            mod3_list.push_back(stats[r].mod_3);
            mod9_list.push_back(stats[r].mod_9);
            mod27_list.push_back(stats[r].mod_27);
            entropy_list.push_back(stats[r].bit_entropy);
            avg_odd_mult_list.push_back(avg_odd_mult);
        }

        std::cout << "--- Pearson Correlation Matrix ---\n";
        std::cout << "Correlation(avg_steps, longest_run_of_1s): " << std::fixed << std::setprecision(4) << pearson_correlation(avg_steps_list, run_1s_list) << "\n";
        std::cout << "Correlation(avg_steps, alternation_score): " << pearson_correlation(avg_steps_list, alt_score_list) << "\n";
        std::cout << "Correlation(avg_steps, hamming_weight)   : " << pearson_correlation(avg_steps_list, hamming_list) << "\n";
        std::cout << "Correlation(avg_steps, avg_v2)           : " << pearson_correlation(avg_steps_list, avg_v2_list) << "\n";
        std::cout << "----------------------------------\n";

        // Call the multiple regression solver
        std::vector<std::string> feature_names = {
            "longest_run_of_1s", "alternation_score", "hamming_weight", "avg_v2",
            "trailing_ones", "trailing_zeros_3n1", "mod_3", "mod_9", "mod_27",
            "bit_entropy", "avg_odd_mult"
        };
        std::vector<std::vector<double>> features = {
            run_1s_list, alt_score_list, hamming_list, avg_v2_list,
            trailing_ones_list, tz_3n1_list, mod3_list, mod9_list, mod27_list,
            entropy_list, avg_odd_mult_list
        };
        multiple_regression(avg_steps_list, features, feature_names);

        std::cout << "Binary Structural correlation data successfully extracted.\n";
        std::cout << "Execution Time: " << duration << " ms\n";
        DataExporter::export_csv("binary_patterns_mod_" + std::to_string(modulo) + "_limit_" + std::to_string(limit) + ".csv", csv.str());
    }
};

} // namespace research
} // namespace collatz
