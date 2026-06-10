#pragma once

#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <sstream>
#include "DataExporter.hpp"

namespace collatz {
namespace research {

class ResidueAnalyzer {
public:
    static void analyze(unsigned long long limit, unsigned long long modulo) {
        std::cout << "\n========================================================\n";
        std::cout << "Research Module 3: Residue Class Analyzer\n";
        std::cout << "Analyzing modulo: " << modulo << " up to limit: " << limit << "\n";
        std::cout << "========================================================\n";

        auto start_time = std::chrono::high_resolution_clock::now();

        struct ClassStats {
            unsigned long long count = 0;
            unsigned long long total_steps = 0;
            unsigned long long max_steps = 0;
            unsigned long long total_v2 = 0;
            unsigned long long total_peak = 0;
        };

        std::vector<ClassStats> stats(modulo);

        for (unsigned long long i = 1; i <= limit; i+=2) {
            unsigned long long current = i;
            unsigned long long peak = i;
            int steps = 0;
            int first_v2 = 0;

            // Compute one Odd-to-Odd step to get v_2
            unsigned long long next_n = 3 * current + 1;
            if (next_n > peak) peak = next_n;
            while ((next_n & 1) == 0) {
                next_n >>= 1;
                first_v2++;
                steps++;
            }
            
            // Just simulate the rest to get stopping time 
            current = next_n;
            while (current != 1) {
                if (current > peak) peak = current;
                if (current & 1) current = 3 * current + 1;
                else current >>= 1;
                steps++;
            }

            unsigned long long r = i % modulo;
            stats[r].count++;
            stats[r].total_steps += steps;
            stats[r].total_v2 += first_v2;
            stats[r].total_peak += peak;
            if (steps > stats[r].max_steps) stats[r].max_steps = steps;
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

        std::stringstream csv;
        csv << "residue,count,avg_stopping_time,max_stopping_time,avg_v2,avg_peak\n";

        std::cout << "Residue classes evaluated: " << modulo << "\n";
        
        std::vector<std::pair<unsigned long long, double>> avg_steps_vec;

        for (unsigned long long r = 0; r < modulo; ++r) {
            if (stats[r].count == 0) continue;
            double avg_steps = (double)stats[r].total_steps / stats[r].count;
            double avg_v2 = (double)stats[r].total_v2 / stats[r].count;
            double avg_peak = (double)stats[r].total_peak / stats[r].count;
            csv << r << "," << stats[r].count << "," << avg_steps << "," << stats[r].max_steps << "," << avg_v2 << "," << avg_peak << "\n";
            avg_steps_vec.push_back({r, avg_steps});
        }

        std::sort(avg_steps_vec.begin(), avg_steps_vec.end(), [](const std::pair<unsigned long long, double>& a, const std::pair<unsigned long long, double>& b) {
            return a.second > b.second; // descending
        });

        std::cout << "\n--- Top 20 Hardest Residue Classes (Highest Avg Steps) ---\n";
        for(size_t i=0; i < std::min((size_t)20, avg_steps_vec.size()); ++i) {
            std::cout << "  Rank " << std::setw(2) << i+1 << ": Modulo " << std::setw(4) << avg_steps_vec[i].first 
                      << " (Avg Steps: " << std::fixed << std::setprecision(2) << avg_steps_vec[i].second << ")\n";
        }

        std::cout << "\n--- Top 20 Easiest Residue Classes (Lowest Avg Steps) ---\n";
        for(size_t i=0; i < std::min((size_t)20, avg_steps_vec.size()); ++i) {
            size_t idx = avg_steps_vec.size() - 1 - i;
            std::cout << "  Rank " << std::setw(2) << i+1 << ": Modulo " << std::setw(4) << avg_steps_vec[idx].first 
                      << " (Avg Steps: " << std::fixed << std::setprecision(2) << avg_steps_vec[idx].second << ")\n";
        }

        std::cout << "\nExecution Time: " << duration << " ms\n";
        DataExporter::export_csv("residue_mod_" + std::to_string(modulo) + "_limit_" + std::to_string(limit) + ".csv", csv.str());
    }
};

} // namespace research
} // namespace collatz
