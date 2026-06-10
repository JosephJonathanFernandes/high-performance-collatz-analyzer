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

class ResidueEvolutionAnalyzer {
public:
    static void analyze(unsigned long long mod_start,
                        unsigned long long mod_end,
                        unsigned long long limit = 10000000)
    {
        std::cout << "\n========================================================\n";
        std::cout << "Research Module 11: Residue Evolution Analyzer\n";
        std::cout << "Moduli : " << mod_start << " → " << mod_end << "\n";
        std::cout << "Limit  : " << limit << " per modulo level\n";
        std::cout << "========================================================\n";

        // Validate: mod_start and mod_end must be powers of 2
        auto is_pow2 = [](unsigned long long v) {
            return v > 0 && (v & (v - 1)) == 0;
        };
        if (!is_pow2(mod_start) || !is_pow2(mod_end) || mod_start > mod_end) {
            std::cerr << "[ERROR] Both moduli must be powers of 2 with mod_start <= mod_end.\n";
            return;
        }

        struct LevelResult {
            unsigned long long modulo;
            unsigned long long hardest_residue;
            double hardest_avg_steps;
            unsigned long long easiest_residue;
            double easiest_avg_steps;
            bool conjecture_2k_minus_1_holds;
            bool conjecture_rk_holds;
        };

        std::vector<LevelResult> results;

        std::stringstream csv;
        csv << "modulo,hardest_residue,hardest_avg_steps,easiest_residue,easiest_avg_steps,"
               "conjecture_2k_minus_1_holds,conjecture_rk_holds\n";

        for (unsigned long long mod = mod_start; mod <= mod_end; mod *= 2) {
            int bit_width = 0;
            { unsigned long long t = mod; while (t > 1) { t >>= 1; bit_width++; } }

            // 2^k - 1 is the conjectured hardest
            unsigned long long conj_hard = mod - 1;
            // (2^k - 1) / 3 — only an integer when k is even or satisfies specific conditions
            unsigned long long conj_easy = 0;
            bool rk_is_integer = ((mod - 1) % 3 == 0);
            if (rk_is_integer) conj_easy = (mod - 1) / 3;

            // --- Simulate ---
            auto t0 = std::chrono::high_resolution_clock::now();

            std::vector<unsigned long long> total_steps(mod, 0);
            std::vector<unsigned long long> count(mod, 0);

            for (unsigned long long i = 1; i <= limit; i += 2) {
                unsigned long long cur = i;
                int steps = 0;
                while (cur != 1) {
                    if (cur & 1) {
                        cur = 3 * cur + 1;
                        while ((cur & 1) == 0) { cur >>= 1; steps++; }
                    } else {
                        cur >>= 1; steps++;
                    }
                }
                unsigned long long r = i % mod;
                total_steps[r] += steps;
                count[r]++;
            }

            auto t1 = std::chrono::high_resolution_clock::now();
            long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

            // Find hardest and easiest
            unsigned long long hard_r = 0, easy_r = 0;
            double hard_avg = -1.0, easy_avg = 1e18;

            for (unsigned long long r = 0; r < mod; ++r) {
                if (count[r] == 0) continue;
                double avg = (double)total_steps[r] / count[r];
                if (avg > hard_avg) { hard_avg = avg; hard_r = r; }
                if (avg < easy_avg) { easy_avg = avg; easy_r = r; }
            }

            bool hard_holds = (hard_r == conj_hard);
            bool easy_holds = rk_is_integer && (easy_r == conj_easy);

            LevelResult lr{mod, hard_r, hard_avg, easy_r, easy_avg,
                           hard_holds, easy_holds};
            results.push_back(lr);

            // --- Console ---
            std::cout << "\nmod " << std::setw(5) << mod
                      << "  (k=" << bit_width << ")  |  Time: " << ms << " ms\n";
            std::cout << "  Hardest  : residue " << std::setw(6) << hard_r
                      << "  avg=" << std::fixed << std::setprecision(2) << hard_avg
                      << "  binary=";
            for (int b = bit_width - 1; b >= 0; --b)
                std::cout << ((hard_r >> b) & 1);
            std::cout << "\n";
            std::cout << "  Easiest  : residue " << std::setw(6) << easy_r
                      << "  avg=" << std::setprecision(2) << easy_avg
                      << "  binary=";
            for (int b = bit_width - 1; b >= 0; --b)
                std::cout << ((easy_r >> b) & 1);
            std::cout << "\n";
            std::cout << "  Conjecture [2^k-1 is hardest] : "
                      << (hard_holds ? "HOLDS ✓" : "FAILS ✗") << "\n";
            if (rk_is_integer)
                std::cout << "  Conjecture [rk=(2^k-1)/3 is easiest] : "
                          << (easy_holds ? "HOLDS ✓" : "FAILS ✗") << "\n";
            else
                std::cout << "  Conjecture [rk=(2^k-1)/3] : NOT INTEGER for k="
                          << bit_width << "\n";

            csv << mod << "," << hard_r << ","
                << std::fixed << std::setprecision(4) << hard_avg << ","
                << easy_r << ","
                << std::setprecision(4) << easy_avg << ","
                << (hard_holds ? 1 : 0) << ","
                << (easy_holds ? 1 : 0) << "\n";
        }

        // Summary
        int hard_score = 0, easy_score = 0, levels = 0;
        for (const auto& r : results) {
            if (r.conjecture_2k_minus_1_holds) hard_score++;
            if (r.conjecture_rk_holds) easy_score++;
            levels++;
        }
        std::cout << "\n--- Conjecture Scorecard ---\n";
        std::cout << "  [2^k-1 is hardest]    : " << hard_score << "/" << levels << " levels\n";
        std::cout << "  [rk is easiest]       : " << easy_score << "/" << levels << " levels\n";
        std::cout << "----------------------------\n";

        DataExporter::export_csv(
            "residue_evolution_" + std::to_string(mod_start) + "_"
            + std::to_string(mod_end) + "_limit_" + std::to_string(limit) + ".csv",
            csv.str());
    }
};

} // namespace research
} // namespace collatz
