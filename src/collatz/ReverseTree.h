#pragma once

#include <iostream>
#include <vector>
#include <unordered_set>
#include <queue>
#include <chrono>

namespace collatz {

/**
 * @brief Analyzes the reverse Collatz tree starting from 1.
 *
 * Demonstrates the exponential branching factor of the inverse Collatz map.
 */
class ReverseTreeAnalyzer {
public:
    static void analyze(int max_depth) {
        std::cout << "\n========================================================\n";
        std::cout << "Research Module 1: Reverse Collatz Tree Generator\n";
        std::cout << "Starting from 1 and building tree to depth: " << max_depth << "\n";
        std::cout << "========================================================\n";

        auto start_time = std::chrono::high_resolution_clock::now();

        std::unordered_set<long long> visited;
        std::queue<std::pair<long long, int>> q;

        q.push({1, 0});
        visited.insert(1);

        long long total_nodes = 0;
        long long max_value_reached = 1;

        std::vector<long long> nodes_per_depth(max_depth + 1, 0);

        while (!q.empty()) {
            auto current = q.front();
            long long node = current.first;
            int depth = current.second;
            q.pop();

            nodes_per_depth[depth]++;
            total_nodes++;
            if (node > max_value_reached) max_value_reached = node;

            if (depth == max_depth) continue;

            long long child1 = node * 2;
            if (visited.find(child1) == visited.end()) {
                visited.insert(child1);
                q.push({child1, depth + 1});
            }

            if ((node - 1) % 3 == 0) {
                long long child2 = (node - 1) / 3;
                if (child2 > 1 && (child2 % 2 != 0)) {
                    if (visited.find(child2) == visited.end()) {
                        visited.insert(child2);
                        q.push({child2, depth + 1});
                    }
                }
            }
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

        for (int d = 0; d <= max_depth; ++d) {
            std::cout << "Depth " << d << ": " << nodes_per_depth[d] << " nodes\n";
        }

        std::cout << "\n--- Reverse Tree Summary ---\n";
        std::cout << "Total Unique Nodes   : " << total_nodes << "\n";
        std::cout << "Max Value Reached    : " << max_value_reached << "\n";
        std::cout << "Execution Time       : " << duration << " ms\n";
    }
};

} // namespace collatz
