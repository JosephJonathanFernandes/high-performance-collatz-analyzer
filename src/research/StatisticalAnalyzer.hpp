#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <sstream>
#include "DataExporter.hpp"

namespace collatz {
namespace research {

class StatisticalAnalyzer {
public:
    static void analyze(unsigned long long limit) {
        std::cout << "\n========================================================\n";
        std::cout << "Research Module 6: Statistical Analysis (v_2 Verification)\n";
        std::cout << "Verifying P(v_2 = k) = 1/2^k up to: " << limit << "\n";
        std::cout << "========================================================\n";

        auto start_time = std::chrono::high_resolution_clock::now();

        unsigned long long total_odds = 0;
        std::vector<unsigned long long> v_2_counts(64, 0); 
        int max_v_2 = 0;

        for (unsigned long long n = 3; n <= limit; n += 2) {
            total_odds++;

            unsigned long long next_n = 3 * n + 1;
            int v_2 = 0;
            
            while ((next_n & 1) == 0) {
                next_n >>= 1;
                v_2++;
            }

            if (v_2 < 64) v_2_counts[v_2]++;
            if (v_2 > max_v_2) max_v_2 = v_2;
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

        std::stringstream csv;
        csv << "v_2,measured_prob,theoretical_prob,error_percentage\n";

        std::cout << std::left << std::setw(5) << "v_2" 
                  << std::setw(15) << "Measured (%)" 
                  << std::setw(18) << "Theoretical (%)" 
                  << std::setw(15) << "Error (%)" << "\n";
        std::cout << "--------------------------------------------------------\n";

        for (int k = 1; k <= max_v_2 && k <= 15; ++k) {
            double measured = (double)v_2_counts[k] / total_odds;
            double theoretical = 1.0 / std::pow(2.0, k);
            double error = std::abs(measured - theoretical) / theoretical * 100.0;

            std::cout << std::left << std::setw(5) << k 
                      << std::setw(15) << std::fixed << std::setprecision(6) << (measured * 100.0) 
                      << std::setw(18) << (theoretical * 100.0) 
                      << std::setw(15) << error << "\n";
                      
            csv << k << "," << measured << "," << theoretical << "," << error << "\n";
        }

        std::cout << "\nExecution Time: " << duration << " ms\n";
        DataExporter::export_csv("statistical_v2_limit_" + std::to_string(limit) + ".csv", csv.str());
    }
};

} // namespace research
} // namespace collatz
