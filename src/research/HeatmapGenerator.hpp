#pragma once

#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <string>
#include "DataExporter.hpp"

namespace collatz {
namespace research {

class HeatmapGenerator {
public:
    static void analyze(unsigned long long modulo,
                        unsigned long long limit = 10000000)
    {
        // modulo must be power of 2
        if ((modulo & (modulo - 1)) != 0 || modulo == 0) {
            std::cerr << "[ERROR] Modulo must be a power of 2.\n";
            return;
        }

        int bit_width = 0;
        { unsigned long long t = modulo; while (t > 1) { t >>= 1; bit_width++; } }

        std::cout << "\n========================================================\n";
        std::cout << "Research Module 13: Collatz Difficulty Heatmap\n";
        std::cout << "Modulo : " << modulo << "  Limit : " << limit << "\n";
        std::cout << "========================================================\n";

        std::vector<unsigned long long> total_steps(modulo, 0);
        std::vector<unsigned long long> total_peak(modulo, 0);
        std::vector<unsigned long long> total_v2(modulo, 0);
        std::vector<unsigned long long> count(modulo, 0);

        auto t0 = std::chrono::high_resolution_clock::now();

        for (unsigned long long i = 1; i <= limit; i += 2) {
            unsigned long long cur = i;
            unsigned long long peak = i;
            int steps = 0, v2_first = 0;

            unsigned long long nx = 3 * cur + 1;
            if (nx > peak) peak = nx;
            while ((nx & 1) == 0) { nx >>= 1; v2_first++; steps++; }
            cur = nx;
            while (cur != 1) {
                if (cur > peak) peak = cur;
                if (cur & 1) {
                    cur = 3 * cur + 1;
                    while ((cur & 1) == 0) { cur >>= 1; steps++; }
                } else {
                    cur >>= 1; steps++;
                }
            }

            unsigned long long r = i % modulo;
            total_steps[r] += steps;
            total_peak[r]  += peak;
            total_v2[r]    += v2_first;
            count[r]++;
        }

        auto t1 = std::chrono::high_resolution_clock::now();
        long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

        // Compute averages
        std::vector<double> avg_steps(modulo, 0.0);
        std::vector<double> avg_peak(modulo, 0.0);
        std::vector<double> avg_v2(modulo, 0.0);

        double max_steps = 0, min_steps = 1e18;
        for (unsigned long long r = 0; r < modulo; ++r) {
            if (count[r] == 0) continue;
            avg_steps[r] = (double)total_steps[r] / count[r];
            avg_peak[r]  = (double)total_peak[r]  / count[r];
            avg_v2[r]    = (double)total_v2[r]    / count[r];
            if (avg_steps[r] > max_steps) max_steps = avg_steps[r];
            if (avg_steps[r] < min_steps) min_steps = avg_steps[r];
        }

        // Determine top 20% and bottom 20% thresholds for DOT coloring
        std::vector<double> sorted_steps;
        for (unsigned long long r = 0; r < modulo; ++r)
            if (count[r] > 0) sorted_steps.push_back(avg_steps[r]);
        std::sort(sorted_steps.begin(), sorted_steps.end());
        double thresh_hard = sorted_steps[(int)(sorted_steps.size() * 0.80)];
        double thresh_easy = sorted_steps[(int)(sorted_steps.size() * 0.20)];

        // Console: top 10 hardest / easiest
        std::vector<std::pair<double, unsigned long long>> ranked;
        for (unsigned long long r = 0; r < modulo; ++r)
            if (count[r] > 0) ranked.push_back({avg_steps[r], r});
        std::sort(ranked.begin(), ranked.end(),
                  [](const auto& a, const auto& b){ return a.first > b.first; });

        std::cout << "\nTop 10 Hardest Residues:\n";
        std::cout << std::left << std::setw(8) << "Residue"
                  << std::setw(12) << "AvgSteps"
                  << std::setw(12) << "AvgPeak"
                  << "Binary\n";
        for (int i = 0; i < std::min(10, (int)ranked.size()); ++i) {
            unsigned long long r = ranked[i].second;
            std::cout << std::setw(8) << r
                      << std::setw(12) << std::fixed << std::setprecision(2) << avg_steps[r]
                      << std::setw(12) << std::setprecision(0) << avg_peak[r];
            for (int b = bit_width - 1; b >= 0; --b) std::cout << ((r >> b) & 1);
            std::cout << "\n";
        }

        std::cout << "\nTop 10 Easiest Residues:\n";
        std::cout << std::left << std::setw(8) << "Residue"
                  << std::setw(12) << "AvgSteps"
                  << std::setw(12) << "AvgPeak"
                  << "Binary\n";
        for (int i = (int)ranked.size() - 1; i >= std::max(0, (int)ranked.size() - 10); --i) {
            unsigned long long r = ranked[i].second;
            std::cout << std::setw(8) << r
                      << std::setw(12) << std::fixed << std::setprecision(2) << avg_steps[r]
                      << std::setw(12) << std::setprecision(0) << avg_peak[r];
            for (int b = bit_width - 1; b >= 0; --b) std::cout << ((r >> b) & 1);
            std::cout << "\n";
        }

        std::cout << "\n  Simulation time : " << ms << " ms\n";

        // ---- CSV exports ----
        std::string mod_str = std::to_string(modulo);

        auto make_csv = [&](const std::vector<double>& vals, const char* col_name) {
            std::stringstream ss;
            ss << "residue," << col_name << "\n";
            for (unsigned long long r = 0; r < modulo; ++r)
                if (count[r] > 0)
                    ss << r << "," << std::fixed << std::setprecision(4) << vals[r] << "\n";
            return ss.str();
        };

        DataExporter::export_csv("heatmap_steps_" + mod_str + ".csv",
                                 make_csv(avg_steps, "avg_stopping_time"));
        DataExporter::export_csv("heatmap_peak_"  + mod_str + ".csv",
                                 make_csv(avg_peak,  "avg_peak"));
        DataExporter::export_csv("heatmap_v2_"    + mod_str + ".csv",
                                 make_csv(avg_v2,    "avg_v2"));

        // ---- Graphviz DOT export ----
        std::stringstream dot;
        dot << "digraph collatz_heatmap_" << modulo << " {\n";
        dot << "  rankdir=LR;\n";
        dot << "  node [shape=box, style=filled, fontsize=8];\n";
        for (unsigned long long r = 0; r < modulo; ++r) {
            if (count[r] == 0) continue;
            const char* color;
            if (avg_steps[r] >= thresh_hard)      color = "\"#e74c3c\""; // red
            else if (avg_steps[r] <= thresh_easy) color = "\"#2ecc71\""; // green
            else                                   color = "\"#f39c12\""; // yellow
            dot << "  r" << r << " [label=\"" << r << "\\n"
                << std::fixed << std::setprecision(1) << avg_steps[r]
                << "\", fillcolor=" << color << "];\n";
        }
        dot << "}\n";

        // Write DOT file to data/csv directory (reuse DataExporter path)
        DataExporter::export_csv("heatmap_" + mod_str + ".dot", dot.str());

        std::cout << "Exported: heatmap_steps, heatmap_peak, heatmap_v2 CSVs + .dot file\n";
    }
};

} // namespace research
} // namespace collatz
