#pragma once

#include <iostream>
#include <vector>
#include <unordered_set>
#include <queue>
#include <chrono>
#include <sstream>
#include "DataExporter.hpp"

namespace collatz {
namespace research {

class ReverseTreeExplorer {
public:
    static void analyze(int max_depth) {
        std::cout << "\n========================================================\n";
        std::cout << "Research Module 1: Reverse Collatz Tree Explorer\n";
        std::cout << "Generating tree up to depth: " << max_depth << "\n";
        std::cout << "========================================================\n";

        auto start_time = std::chrono::high_resolution_clock::now();

        std::vector<unsigned long long> nodes_per_depth(max_depth + 1, 0);
        std::vector<unsigned long long> pruned_per_depth(max_depth + 1, 0);
        nodes_per_depth[0] = 1;
        unsigned long long total_nodes = 1;
        unsigned long long total_pruned = 0;
        bool overflow_detected = false;

        // DFS using an explicit stack to avoid function call overhead for billions of nodes
        // Stack stores pairs of: {node, depth}
        std::vector<std::pair<unsigned long long, int>> stack;
        // Pre-allocate to max_depth to prevent reallocations
        stack.reserve(max_depth + 1); 
        stack.push_back({1ULL, 0});

        while (!stack.empty()) {
            auto current = stack.back();
            stack.pop_back();

            unsigned long long node = current.first;
            int depth = current.second;

            if (depth == max_depth) continue;

            // Child 2: (node - 1) / 3
            // Pushed first so Child 1 is popped first (mimics standard DFS)
            if (node > 4 && (node % 3 == 1)) {
                unsigned long long child2 = (node - 1) / 3;
                if (child2 > 1 && (child2 % 2 != 0)) {
                    nodes_per_depth[depth + 1]++;
                    total_nodes++;
                    stack.push_back({child2, depth + 1});
                }
            }

            // Child 1: node * 2
            unsigned long long child1 = node * 2;
            if (child1 < node) {
                overflow_detected = true; 
                pruned_per_depth[depth + 1]++;
                total_pruned++;
            } else {
                nodes_per_depth[depth + 1]++;
                total_nodes++;
                stack.push_back({child1, depth + 1});
            }
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

        // Print & Export
        std::stringstream csv;
        csv << "depth,nodes,branching_factor,pruned_branches\n";

        double avg_branching_factor = 0;
        int valid_branch_depths = 0;

        for (int d = 0; d <= max_depth; ++d) {
            double bf = (d > 0 && nodes_per_depth[d-1] > 0) ? (double)nodes_per_depth[d] / nodes_per_depth[d-1] : 0.0;
            if (d > 4) {
                avg_branching_factor += bf;
                valid_branch_depths++;
            }
            csv << d << "," << nodes_per_depth[d] << "," << bf << "," << pruned_per_depth[d] << "\n";
        }

        if (valid_branch_depths > 0) avg_branching_factor /= valid_branch_depths;

        std::cout << "\n--- Reverse Tree Summary ---\n";
        std::cout << "Total Unique Nodes   : " << total_nodes << "\n";
        std::cout << "Avg Branching Factor : " << avg_branching_factor << "\n";
        if (overflow_detected) {
            std::cout << "[WARNING] Integer overflow detected at extreme depth.\n";
            std::cout << "Total Branches Pruned: " << total_pruned << "\n";
        }
        std::cout << "Execution Time       : " << duration << " ms\n";

        DataExporter::export_csv("reverse_tree_" + std::to_string(max_depth) + ".csv", csv.str());
    }
};

} // namespace research
} // namespace collatz
