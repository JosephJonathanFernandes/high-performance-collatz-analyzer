#pragma once

#include <iostream>
#include <vector>
#include <queue>
#include <chrono>
#include <iomanip>
#include <sstream>
#include "DataExporter.hpp"

namespace collatz {
namespace research {

struct SequenceRecord {
    unsigned long long starting_number;
    double ratio_r;
    unsigned long long peak;
    int steps;

    bool operator>(const SequenceRecord& other) const {
        return ratio_r > other.ratio_r; // Min-heap based on R(n)
    }
};

class NearCounterexampleDetector {
public:
    static void analyze(unsigned long long limit, int top_k = 100) {
        std::cout << "\n========================================================\n";
        std::cout << "Research Module 4: Near Counterexample Detector\n";
        std::cout << "Tracking top " << top_k << " highest R(n) values up to: " << limit << "\n";
        std::cout << "========================================================\n";

        auto start_time = std::chrono::high_resolution_clock::now();

        std::priority_queue<SequenceRecord, std::vector<SequenceRecord>, std::greater<SequenceRecord>> min_heap;

        for (unsigned long long i = 1; i <= limit; ++i) {
            unsigned long long current = i;
            unsigned long long peak = current;
            int steps = 0;

            while (current != 1) {
                if (current & 1) {
                    current = 3 * current + 1;
                } else {
                    current >>= 1;
                }
                if (current > peak) peak = current;
                steps++;
            }

            double ratio = (double)peak / (double)i;

            if (min_heap.size() < (size_t)top_k) {
                min_heap.push({i, ratio, peak, steps});
            } else if (ratio > min_heap.top().ratio_r) {
                min_heap.pop();
                min_heap.push({i, ratio, peak, steps});
            }
        }

        // Extract from heap and sort descending
        std::vector<SequenceRecord> top_records;
        while (!min_heap.empty()) {
            top_records.push_back(min_heap.top());
            min_heap.pop();
        }
        std::reverse(top_records.begin(), top_records.end());

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

        std::stringstream csv;
        csv << "rank,starting_number,ratio_r,peak_value,steps\n";

        std::cout << "Top 5 Near Counterexamples [R(n) = Peak / Start]:\n";
        for (int i = 0; i < std::min(5, (int)top_records.size()); ++i) {
            const auto& rec = top_records[i];
            std::cout << "  Rank " << i+1 << ": n=" << rec.starting_number 
                      << " | R(n)=" << std::fixed << std::setprecision(2) << rec.ratio_r 
                      << " | Peak=" << rec.peak << " | Steps=" << rec.steps << "\n";
        }

        for (int i = 0; i < (int)top_records.size(); ++i) {
            const auto& rec = top_records[i];
            csv << (i+1) << "," << rec.starting_number << "," << rec.ratio_r << "," << rec.peak << "," << rec.steps << "\n";
        }

        std::cout << "Execution Time: " << duration << " ms\n";
        DataExporter::export_csv("near_counterexamples_limit_" + std::to_string(limit) + ".csv", csv.str());
    }
};

} // namespace research
} // namespace collatz
