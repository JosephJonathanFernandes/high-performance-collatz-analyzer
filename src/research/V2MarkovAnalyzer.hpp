#pragma once

#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cmath>
#include <map>
#include <unordered_set>
#include "DataExporter.hpp"

namespace collatz {
namespace research {

// ============================================================
// Module 19: v2 Transition Matrix Analyzer
//
// Calculates the global Markov transition matrix P(v2(t) | v2(t-1))
// across the Collatz state space.
// Uses a visited set to count only *unique* odd-to-odd transitions.
// ============================================================

class V2MarkovAnalyzer {
public:
    static void analyze(unsigned long long limit) {
        std::cout << "\n========================================================\n";
        std::cout << "Research Module 19: v2 Transition Matrix Analyzer\n";
        std::cout << "Limit : " << limit << "\n";
        std::cout << "Mode  : Unique Transitions (Caching to avoid overlapping paths)\n";
        std::cout << "========================================================\n\n";

        auto t0 = std::chrono::high_resolution_clock::now();

        std::unordered_set<unsigned long long> visited;
        std::map<int, std::map<int, unsigned long long>> transitions;
        std::map<int, unsigned long long> state_counts;

        for (unsigned long long i = 1; i <= limit; i += 2) {
            unsigned long long current = i;
            
            // To record a transition, we need the previous v2
            int prev_v2 = -1;

            while (current != 1) {
                if (visited.count(current)) {
                    break; // Path merges with already fully-explored path
                }
                visited.insert(current);

                // Ensure we are on an odd number
                if (current % 2 == 1) {
                    current = 3 * current + 1;
                    int v2 = 0;
                    while (current % 2 == 0 && current > 0) {
                        current /= 2;
                        v2++;
                    }

                    if (prev_v2 != -1) {
                        transitions[prev_v2][v2]++;
                        state_counts[prev_v2]++;
                    }
                    prev_v2 = v2;
                } else {
                    // Shouldn't happen based on loop structure, but for safety
                    current /= 2;
                }
            }
        }

        auto t1 = std::chrono::high_resolution_clock::now();
        long long sim_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

        std::cout << "--- Global Markov Transition Matrix P(v2_next | v2_current) ---\n";
        std::cout << "If purely random drift, P(v2_next=k) should be ~ (0.5)^k independently.\n\n";

        std::stringstream csv;
        csv << "v2_current,v2_next,count,probability_pct,theoretical_pct\n";

        for (int v_curr = 1; v_curr <= 10; ++v_curr) {
            if (state_counts.find(v_curr) == state_counts.end() || state_counts[v_curr] == 0) {
                continue;
            }

            unsigned long long total_from_curr = state_counts[v_curr];
            std::cout << "From v2 = " << v_curr << " (Total Unique Transitions: " << total_from_curr << ")\n";

            for (int v_next = 1; v_next <= 10; ++v_next) {
                unsigned long long count_next = transitions[v_curr][v_next];
                double prob = (double)count_next / total_from_curr * 100.0;
                double theo_prob = std::pow(0.5, v_next) * 100.0;
                
                std::cout << "  -> v2 = " << std::left << std::setw(2) << v_next 
                          << " : " << std::setw(10) << count_next << " times "
                          << "(" << std::fixed << std::setprecision(2) << std::setw(6) << prob << "%) "
                          << "[Theo: " << std::setprecision(2) << std::setw(6) << theo_prob << "%]\n";
                
                csv << v_curr << "," << v_next << "," << count_next << "," 
                    << std::fixed << std::setprecision(4) << prob << ","
                    << theo_prob << "\n";
            }
        }

        std::cout << "\n--- Scientific Interpretation ---\n";
        std::cout << "  If global transitions deviate significantly from theoretical expectations,\n";
        std::cout << "  (e.g., P(v2=1 | v2=1) > 50%), this reveals a systemic 'memory effect'.\n";
        std::cout << "  This Markov dependence explains why the simple R^2 of the drift model\n";
        std::cout << "  is ~0.94 rather than 1.00.\n\n";

        std::cout << "  Simulation time : " << sim_ms << " ms\n";
        std::cout << "  Unique paths tracked: " << visited.size() << "\n";

        DataExporter::export_csv(
            "global_v2_transitions_limit_" + std::to_string(limit) + ".csv",
            csv.str());
    }
};

} // namespace research
} // namespace collatz
