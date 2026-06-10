#pragma once

#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <cmath>
#include <sstream>
#include "DataExporter.hpp"

namespace collatz {
namespace research {

class OddToOddAnalyzer {
public:
    static void analyze(unsigned long long limit) {
        std::cout << "\n========================================================\n";
        std::cout << "Research Module 2: Compressed Odd-to-Odd Map Analyzer\n";
        std::cout << "Evaluating T(n) = (3n+1) / 2^(v_2) up to: " << limit << "\n";
        std::cout << "========================================================\n";

        auto start_time = std::chrono::high_resolution_clock::now();

        unsigned long long contractions = 0;
        unsigned long long expansions = 0;  
        unsigned long long equals = 0;      
        unsigned long long total_odds = 0;

        std::vector<unsigned long long> v_2_counts(100, 0); 
        int max_v_2 = 0;
        
        double total_log_drift = 0.0;
        double max_multiplier = 0.0;
        unsigned long long max_multiplier_node = 0;

        std::stringstream csv;
        csv << "n,T_n,v_2,multiplier,log_drift\n";

        for (unsigned long long n = 3; n <= limit; n += 2) {
            total_odds++;

            unsigned long long next_n = 3 * n + 1;
            int v_2 = 0;
            
            while ((next_n & 1) == 0) {
                next_n >>= 1;
                v_2++;
            }

            if (v_2 < 100) v_2_counts[v_2]++;
            if (v_2 > max_v_2) max_v_2 = v_2;

            double multiplier = (double)next_n / (double)n;
            double log_drift = std::log(multiplier);
            total_log_drift += log_drift;

            if (multiplier > max_multiplier) {
                max_multiplier = multiplier;
                max_multiplier_node = n;
            }

            if (next_n < n) contractions++;
            else if (next_n > n) expansions++;
            else equals++;

            // Only export sample of data to prevent massive CSV files if limit > 1M
            if (limit <= 1000000 || n % 1000 == 1) {
                csv << n << "," << next_n << "," << v_2 << "," << multiplier << "," << log_drift << "\n";
            }
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

        double expected_drift = total_log_drift / total_odds;

        std::cout << "Total odd numbers analyzed : " << total_odds << "\n";
        std::cout << "Expansions [T(n) > n]      : " << expansions << " (" 
                  << std::fixed << std::setprecision(2) << ((double)expansions / total_odds) * 100.0 << "%)\n";
        std::cout << "Contractions [T(n) < n]    : " << contractions << " (" 
                  << std::fixed << std::setprecision(2) << ((double)contractions / total_odds) * 100.0 << "%)\n";
        std::cout << "Average Logarithmic Drift  : " << std::fixed << std::setprecision(6) << expected_drift << "\n";
        std::cout << "Max Expansion Multiplier   : " << max_multiplier << "x (at n = " << max_multiplier_node << ")\n";
        std::cout << "Execution Time             : " << duration << " ms\n";

        DataExporter::export_csv("odd_to_odd_" + std::to_string(limit) + ".csv", csv.str());
    }
};

} // namespace research
} // namespace collatz
