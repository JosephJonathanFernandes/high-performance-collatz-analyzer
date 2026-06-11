#pragma once

#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cmath>
#include <string>
#include "DataExporter.hpp"

namespace collatz {
namespace research {

// ============================================================
// Module 18: Theorem Checker
//
// Automatically tests symbolic families of numbers for increasing k:
//  - 2^k - 1
//  - (2^k - 1) / 3 (when valid)
//  - 2^k + 1
//  - (2^k + 1) / 3 (when valid)
//
// Computes exact initial v2, average v2 across trajectory,
// exact drift, and stopping-time statistics to bridge computation
// with analytic mathematical explanation.
// ============================================================

class TheoremCheckAnalyzer {
public:
    static void analyze(int max_k = 60) {
        if (max_k > 62) {
            std::cout << "[WARNING] max_k > 62 may overflow 64-bit unsigned integers. Clamping to 62.\n";
            max_k = 62;
        }

        std::cout << "\n========================================================\n";
        std::cout << "Research Module 18: Symbolic Theorem Checker\n";
        std::cout << "Max k : " << max_k << "\n";
        std::cout << "========================================================\n\n";

        auto t0 = std::chrono::high_resolution_clock::now();

        std::stringstream csv;
        csv << "family,k,n,initial_v2,total_steps,avg_v2,avg_log_drift\n";

        // Family 1: 2^k - 1
        std::cout << "--- Family: 2^k - 1 (Mersenne-like, Hardest) ---\n";
        std::cout << "Analytical Expectation: v2(3n+1) = 1 (Maximal odd-to-odd expansion)\n";
        analyze_family("2^k - 1", max_k, csv, [](int k, unsigned long long& n) {
            n = (1ULL << k) - 1;
            return k > 1; // Need odd, k=1 -> n=1 (trivial)
        });

        // Family 2: (2^k - 1) / 3
        std::cout << "\n--- Family: (2^k - 1)/3 (Alternating bits, Easiest) ---\n";
        std::cout << "Analytical Expectation: v2(3n+1) = k (Maximal contraction)\n";
        analyze_family("(2^k - 1)/3", max_k, csv, [](int k, unsigned long long& n) {
            unsigned long long numerator = (1ULL << k) - 1;
            if (numerator % 3 == 0) {
                n = numerator / 3;
                return (n % 2 == 1 && n > 1); // Only care about odd numbers > 1
            }
            return false;
        });

        // Family 3: 2^k + 1
        std::cout << "\n--- Family: 2^k + 1 ---\n";
        analyze_family("2^k + 1", max_k, csv, [](int k, unsigned long long& n) {
            n = (1ULL << k) + 1;
            return k > 0 && (n % 2 == 1);
        });

        // Family 4: (2^k + 1) / 3
        std::cout << "\n--- Family: (2^k + 1)/3 ---\n";
        analyze_family("(2^k + 1)/3", max_k, csv, [](int k, unsigned long long& n) {
            unsigned long long numerator = (1ULL << k) + 1;
            if (numerator % 3 == 0) {
                n = numerator / 3;
                return (n % 2 == 1 && n > 1);
            }
            return false;
        });

        auto t1 = std::chrono::high_resolution_clock::now();
        long long sim_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

        std::cout << "\n  Simulation time : " << sim_ms << " ms\n";

        DataExporter::export_csv(
            "theorem_check_k_" + std::to_string(max_k) + ".csv",
            csv.str());
    }

private:
    template<typename Func>
    static void analyze_family(const std::string& family_name, int max_k, std::stringstream& csv, Func generator) {
        
        std::cout << std::left << std::setw(4) << "k"
                  << std::setw(20) << "n"
                  << std::setw(12) << "Init v2"
                  << std::setw(12) << "Avg v2"
                  << std::setw(15) << "Avg Drift"
                  << "Total Steps\n";
        std::cout << std::string(75, '-') << "\n";

        const double LOG2 = std::log(2.0);
        const double LOG3 = std::log(3.0);

        for (int k = 1; k <= max_k; ++k) {
            unsigned long long n = 0;
            if (!generator(k, n)) {
                continue; // invalid or trivial for this k
            }

            unsigned long long current = n;
            int total_steps = 0;
            
            // Initial v2 calculation
            unsigned long long first_step = 3 * current + 1;
            int initial_v2 = 0;
            while (first_step % 2 == 0 && first_step > 0) {
                first_step /= 2;
                initial_v2++;
            }

            // Full trajectory calculation
            int odd_steps = 0;
            double sum_v2 = 0.0;
            double sum_drift = 0.0;

            while (current != 1 && current != 0) {
                if (current % 2 == 1) {
                    current = 3 * current + 1;
                    total_steps++;
                    int v2 = 0;
                    while (current % 2 == 0 && current > 0) {
                        current /= 2;
                        v2++;
                        total_steps++;
                    }
                    sum_v2 += v2;
                    sum_drift += (LOG3 - v2 * LOG2);
                    odd_steps++;
                } else {
                    current /= 2;
                    total_steps++;
                }
            }

            double avg_v2 = (odd_steps > 0) ? (sum_v2 / odd_steps) : 0.0;
            double avg_drift = (odd_steps > 0) ? (sum_drift / odd_steps) : 0.0;

            std::cout << std::left << std::setw(4) << k
                      << std::setw(20) << n
                      << std::setw(12) << initial_v2
                      << std::fixed << std::setprecision(4) << std::setw(12) << avg_v2
                      << std::fixed << std::setprecision(4) << std::setw(15) << avg_drift
                      << total_steps << "\n";

            csv << family_name << "," << k << "," << n << "," << initial_v2 << ","
                << total_steps << "," << std::fixed << std::setprecision(6) << avg_v2 << ","
                << avg_drift << "\n";
        }
    }
};

} // namespace research
} // namespace collatz
