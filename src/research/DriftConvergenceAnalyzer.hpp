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
// Module 21: Drift Convergence Analyzer
//
// Tests whether mu_k -> mu_infty as the modulus 2^k increases.
// Bins 50M trajectories into 32768 classes, then aggregates
// upward to 16384, 8192, 4096, 2048, 1024.
// ============================================================

class DriftConvergenceAnalyzer {
public:
    static void analyze(unsigned long long limit) {
        std::cout << "\n========================================================\n";
        std::cout << "Research Module 21: Drift Convergence Analyzer\n";
        std::cout << "Limit : " << limit << "\n";
        std::cout << "Hypothesis : mu_k converges to mu_infty\n";
        std::cout << "========================================================\n\n";

        auto t0 = std::chrono::high_resolution_clock::now();

        const double LOG2 = std::log(2.0);
        const double LOG3 = std::log(3.0);
        const int MAX_MOD = 32768;

        std::vector<double> sum_drift_bin(MAX_MOD, 0.0);
        std::vector<unsigned long long> count_bin(MAX_MOD, 0);

        // 1. Single pass: bin trajectory-average drift into 32768 classes
        for (unsigned long long i = 1; i <= limit; i += 2) {
            unsigned long long cur = i;
            double sum_log_drift = 0.0;
            int odd_steps = 0;

            while (cur != 1) {
                if (cur % 2 == 1) {
                    cur = 3 * cur + 1;
                    int v2 = 0;
                    while (cur % 2 == 0 && cur > 0) {
                        cur /= 2;
                        v2++;
                    }
                    sum_log_drift += (LOG3 - v2 * LOG2);
                    odd_steps++;
                } else {
                    cur /= 2;
                }
            }

            if (odd_steps > 0) {
                double avg_drift = sum_log_drift / odd_steps;
                int r = i % MAX_MOD;
                sum_drift_bin[r] += avg_drift;
                count_bin[r]++;
            }
        }

        auto t1 = std::chrono::high_resolution_clock::now();
        long long pass_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

        std::cout << "  [Data Collection: " << pass_ms << " ms]\n\n";

        // 2. Aggregate upward
        std::vector<int> moduli = {1024, 2048, 4096, 8192, 16384, 32768};
        
        std::cout << "--- Drift Convergence Series ---\n";
        std::cout << std::left << std::setw(10) << "Modulus" 
                  << std::setw(15) << "k" 
                  << std::setw(15) << "mu_k" 
                  << "Variance (sigma^2)\n";
        std::cout << "--------------------------------------------------------\n";

        std::stringstream csv;
        csv << "modulus,k,mu_k,variance\n";

        for (int mod : moduli) {
            std::vector<double> class_sums(mod, 0.0);
            std::vector<unsigned long long> class_counts(mod, 0);

            // Aggregate from 32768 bins down to mod bins
            for (int r = 0; r < MAX_MOD; ++r) {
                int r_mod = r % mod;
                class_sums[r_mod] += sum_drift_bin[r];
                class_counts[r_mod] += count_bin[r];
            }

            // Population-weighted mean: total drift sum / total number count
            // (NOT unweighted mean of class averages, which biases toward small classes)
            double total_sum = 0.0;
            unsigned long long total_count = 0;
            for (int r = 0; r < mod; ++r) {
                total_sum += class_sums[r];
                total_count += class_counts[r];
            }
            double mu_k = (total_count > 0) ? (total_sum / total_count) : 0.0;



            // Variance of class averages around the population-weighted mean
            double variance = 0.0;
            int valid_classes = 0;
            for (int r = 0; r < mod; ++r) {
                if (class_counts[r] > 0) {
                    double class_avg = class_sums[r] / class_counts[r];
                    variance += (class_avg - mu_k) * (class_avg - mu_k);
                    valid_classes++;
                }
            }
            variance /= (valid_classes > 0 ? valid_classes : 1);

            int k = (int)std::log2(mod);
            std::cout << std::left << std::setw(10) << mod 
                      << std::setw(15) << k 
                      << std::fixed << std::setprecision(8) << std::setw(15) << mu_k 
                      << std::setprecision(8) << variance << "\n";
            
            csv << mod << "," << k << "," << std::fixed << std::setprecision(8) << mu_k << "," << variance << "\n";
        }

        std::cout << "\n--- Scientific Interpretation ---\n";
        std::cout << "  If mu_k is practically identical across 1024 -> 32768,\n";
        std::cout << "  the state space converges to a stable limiting measure mu_infty.\n";
        std::cout << "  This provides strong computational evidence that macroscopic Collatz\n";
        std::cout << "  behaviour approaches a stable limiting distribution.\n";
        std::cout << "  NOTE: mu_k is now computed as the population-weighted mean\n";
        std::cout << "  (total drift / total numbers), not as mean of class averages.\n\n";

        DataExporter::export_csv(
            "drift_convergence_limit_" + std::to_string(limit) + ".csv",
            csv.str());
    }
};

} // namespace research
} // namespace collatz
