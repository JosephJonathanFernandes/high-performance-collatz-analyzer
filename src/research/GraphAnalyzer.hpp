#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <chrono>

namespace collatz {
namespace research {

class GraphAnalyzer {
public:
    static void analyze(unsigned long long limit) {
        std::cout << "\n========================================================\n";
        std::cout << "Research Module 5: Graph Analysis (Cycle Detection)\n";
        std::cout << "Building odd-to-odd directed graph up to: " << limit << "\n";
        std::cout << "========================================================\n";

        auto start_time = std::chrono::high_resolution_clock::now();

        // Very basic bounded cycle detection using a visited set
        std::unordered_map<unsigned long long, int> state; // 0 = unvisited, 1 = visiting, 2 = visited
        bool non_trivial_cycle_found = false;

        unsigned long long nodes_processed = 0;

        for (unsigned long long i = 3; i <= limit; i += 2) {
            unsigned long long current = i;
            
            // DFS path
            std::vector<unsigned long long> path;

            while (current != 1) {
                if (state[current] == 1) {
                    // Back edge found!
                    if (current != 1 && current != 2 && current != 4) {
                        non_trivial_cycle_found = true;
                        std::cout << "[URGENT] Non-trivial cycle detected involving node: " << current << "\n";
                    }
                    break;
                }
                if (state[current] == 2) {
                    break; // Already resolved
                }

                state[current] = 1; // Mark visiting
                path.push_back(current);

                if (current & 1) {
                    unsigned long long next_n = 3 * current + 1;
                    // Detect overflow immediately
                    if (next_n < current) {
                        break; 
                    }
                    current = next_n;
                } else {
                    current >>= 1;
                }
            }

            for (unsigned long long node : path) {
                state[node] = 2; // Mark resolved
            }
            nodes_processed++;
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

        std::cout << "Graph Nodes Traversed: " << nodes_processed << "\n";
        if (non_trivial_cycle_found) {
            std::cout << "Result: NON-TRIVIAL CYCLE EXISTENCE PROVED (or overflow occurred).\n";
        } else {
            std::cout << "Result: No non-trivial cycles detected within bounds.\n";
        }

        std::cout << "Execution Time: " << duration << " ms\n";
    }
};

} // namespace research
} // namespace collatz
