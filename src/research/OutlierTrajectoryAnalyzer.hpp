#pragma once

#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cmath>
#include <map>
#include <algorithm>
#include "CollatzFeatureEngine.hpp"
#include "DataExporter.hpp"

namespace collatz {
namespace research {

// ============================================================
// Module 17: Outlier Trajectory Analyzer
//
// Analyzes the trajectory of a specific outlier number n.
// Extracts the v2 sequence (number of even divisions per odd step),
// computes the probability distribution of v2, and the Markov
// transition matrix P(v2(t) | v2(t-1)). 
// Export results to CSV to show deviations from random drift.
// ============================================================

class OutlierTrajectoryAnalyzer {
public:
    static void analyze(unsigned long long n) {
        std::cout << "\n========================================================\n";
        std::cout << "Research Module 17: Outlier Trajectory Analyzer\n";
        std::cout << "Number to Analyze : " << n << "\n";
        std::cout << "========================================================\n\n";

        auto t0 = std::chrono::high_resolution_clock::now();

        std::vector<unsigned long long> trajectory;
        std::vector<int> v2_seq;

        unsigned long long current = n;
        unsigned long long peak = current;
        int total_steps = 0;

        // Evolve until 1
        while (current != 1) {
            if (current > peak) peak = current;

            if (current % 2 == 1) {
                trajectory.push_back(current);
                current = 3 * current + 1;
                total_steps++;
                if (current > peak) peak = current;

                int v2 = 0;
                while (current % 2 == 0) {
                    current /= 2;
                    v2++;
                    total_steps++;
                }
                v2_seq.push_back(v2);
            } else {
                current /= 2;
                total_steps++;
            }
        }

        auto t1 = std::chrono::high_resolution_clock::now();
        long long sim_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

        int odd_steps = v2_seq.size();
        if (odd_steps == 0) {
            std::cout << "  No odd steps in trajectory.\n";
            return;
        }

        // Calculate distribution
        std::map<int, int> v2_counts;
        double sum_v2 = 0.0;
        for (int v : v2_seq) {
            v2_counts[v]++;
            sum_v2 += v;
        }
        double avg_v2 = sum_v2 / odd_steps;

        // Calculate Markov transitions
        std::map<int, std::map<int, int>> transitions;
        for (size_t i = 0; i < v2_seq.size() - 1; ++i) {
            transitions[v2_seq[i]][v2_seq[i+1]]++;
        }

        // --- Console Output ---
        std::cout << "--- Trajectory Summary ---\n";
        std::cout << "  Start Number : " << n << "\n";
        std::cout << "  Peak Value   : " << peak << "\n";
        std::cout << "  Total Steps  : " << total_steps << "\n";
        std::cout << "  Odd Steps    : " << odd_steps << "\n";
        std::cout << "  Average v2   : " << std::fixed << std::setprecision(4) << avg_v2 
                  << " (Theoretical ~2.0)\n\n";

        std::cout << "--- v2 Distribution vs Theoretical ---\n";
        std::cout << std::left << std::setw(6) << "v2" 
                  << std::setw(12) << "Count" 
                  << std::setw(15) << "Observed %" 
                  << "Theoretical %\n";
        std::cout << std::string(50, '-') << "\n";
        for (auto const& pair : v2_counts) {
            int v = pair.first;
            int count = pair.second;
            double obs_pct = (double)count / odd_steps * 100.0;
            double theo_pct = std::pow(0.5, v) * 100.0;
            std::cout << std::left << std::setw(6) << v 
                      << std::setw(12) << count 
                      << std::fixed << std::setprecision(1) << std::setw(14) << obs_pct << "%"
                      << std::fixed << std::setprecision(1) << theo_pct << "%\n";
        }
        std::cout << "\n";

        std::cout << "--- Markov Transition Matrix P(v2_next | v2_current) ---\n";
        std::cout << "If purely random drift, P(v2_next=k) should be ~ (0.5)^k independently.\n\n";
        
        std::stringstream csv;
        csv << "v2_current,v2_next,count,probability_pct,theoretical_pct\n";

        for (auto const& curr_pair : v2_counts) {
            int v_curr = curr_pair.first;
            if (transitions.find(v_curr) == transitions.end()) continue;
            
            int total_from_curr = 0;
            for (auto const& next_pair : transitions[v_curr]) {
                total_from_curr += next_pair.second;
            }

            std::cout << "From v2 = " << v_curr << " (Total: " << total_from_curr << ")\n";
            for (auto const& next_pair : transitions[v_curr]) {
                int v_next = next_pair.first;
                int count_next = next_pair.second;
                double prob = (double)count_next / total_from_curr * 100.0;
                double theo_prob = std::pow(0.5, v_next) * 100.0;
                
                std::cout << "  -> v2 = " << std::left << std::setw(2) << v_next 
                          << " : " << std::setw(5) << count_next << " times "
                          << "(" << std::fixed << std::setprecision(1) << std::setw(5) << prob << "%) "
                          << "[Theo: " << std::setw(5) << theo_prob << "%]\n";
                
                csv << v_curr << "," << v_next << "," << count_next << "," 
                    << std::fixed << std::setprecision(2) << prob << ","
                    << theo_prob << "\n";
            }
        }

        std::cout << "\n--- Scientific Interpretation ---\n";
        std::cout << "  If P(v2=1 | v2=1) is significantly > 50%, then the sequence of operations\n";
        std::cout << "  tends to get 'trapped' in a low-v2 state. This means the number grows much\n";
        std::cout << "  faster than the drift model predicts, explaining why it is an outlier.\n\n";

        std::cout << "  Simulation time : " << sim_ms << " ms\n";

        DataExporter::export_csv(
            "outlier_transitions_" + std::to_string(n) + ".csv",
            csv.str());
    }
};

} // namespace research
} // namespace collatz
