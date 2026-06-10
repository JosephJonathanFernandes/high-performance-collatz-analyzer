#pragma once

#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <string>
#include "CollatzFeatureEngine.hpp"
#include "DataExporter.hpp"

namespace collatz {
namespace research {

class AdvancedBinaryAnalyzer {
public:
    static void analyze(unsigned long long limit, unsigned long long modulo) {
        if ((modulo & (modulo - 1)) != 0 || modulo == 0) {
            std::cerr << "[ERROR] Modulo must be a power of 2.\n";
            return;
        }

        int bit_width = 0;
        { unsigned long long t = modulo; while (t > 1) { t >>= 1; bit_width++; } }

        std::cout << "\n========================================================\n";
        std::cout << "Research Module 14: Advanced Binary Structure Analyzer\n";
        std::cout << "Modulo : " << modulo << " (" << bit_width << " bits)\n";
        std::cout << "Limit  : " << limit << "\n";
        std::cout << "Features: 19 binary + arithmetic\n";
        std::cout << "========================================================\n";

        // Extended feature struct (11 existing + 8 new)
        struct ExtFeatures {
            // Existing 11
            int max_run_1s, max_run_0s, hamming_weight, alternation_score;
            int trailing_ones, trailing_zeros_3n1;
            int mod_3, mod_9, mod_27;
            double bit_entropy, avg_odd_mult;

            // 8 new
            int leading_ones;
            int leading_zeros;
            int trailing_zeros;           // trailing zeros of n itself
            int longest_alternating_run;
            int block_count;              // number of contiguous same-bit runs
            double run_length_entropy;    // Shannon entropy of run-length distribution
            double binary_compressibility; // bit_width / block_count
            int max_run_0s_dup;           // promoted: longest run of 0s
        };

        auto extract_ext = [&](unsigned long long r) -> ExtFeatures {
            ExtFeatures f{};
            f.mod_3 = r % 3; f.mod_9 = r % 9; f.mod_27 = r % 27;

            // Pass 1: standard metrics (LSB → MSB)
            int run1 = 0, max1 = 0, run0 = 0, max0 = 0;
            int weight = 0, alts = 0, last_bit = -1;
            int trail1 = 0, trail0 = 0;
            bool still_t1 = true, still_t0 = true;

            for (int i = 0; i < bit_width; ++i) {
                int bit = (r >> i) & 1;
                if (bit == 1) {
                    weight++; run1++;
                    if (run1 > max1) max1 = run1;
                    run0 = 0; still_t0 = false;
                    if (still_t1) trail1++;
                } else {
                    run0++;
                    if (run0 > max0) max0 = run0;
                    run1 = 0; still_t1 = false;
                    if (still_t0) trail0++;
                }
                if (last_bit != -1 && bit != last_bit) alts++;
                last_bit = bit;
            }

            f.max_run_1s = max1; f.max_run_0s = max0;
            f.hamming_weight = weight; f.alternation_score = alts;
            f.trailing_ones = trail1; f.trailing_zeros = trail0;
            f.max_run_0s_dup = max0;

            // 3n+1 trailing zeros
            unsigned long long rn = 3 * r + 1;
            int tz = 0;
            while ((rn & 1) == 0 && rn > 0) { tz++; rn >>= 1; }
            f.trailing_zeros_3n1 = tz;

            // Bit entropy
            double p1 = (bit_width > 0) ? (double)weight / bit_width : 0.0;
            double p0 = 1.0 - p1;
            double ent = 0.0;
            if (p1 > 0) ent -= p1 * std::log2(p1);
            if (p0 > 0) ent -= p0 * std::log2(p0);
            f.bit_entropy = ent;

            // Leading ones and zeros (from MSB down)
            int lead1 = 0, lead0 = 0;
            bool lz = true, lo = true;
            for (int i = bit_width - 1; i >= 0; --i) {
                int bit = (r >> i) & 1;
                if (lz && bit == 0) lead0++;
                else lz = false;
                if (lo && bit == 1) lead1++;
                else lo = false;
            }
            f.leading_ones  = lead1;
            f.leading_zeros = lead0;

            // Longest alternating run
            int best_alt = 1, cur_alt = 1;
            for (int i = 1; i < bit_width; ++i) {
                if (((r >> i) & 1) != ((r >> (i-1)) & 1)) {
                    cur_alt++;
                    if (cur_alt > best_alt) best_alt = cur_alt;
                } else {
                    cur_alt = 1;
                }
            }
            f.longest_alternating_run = best_alt;

            // Block count + run-length entropy
            // Collect all run lengths
            std::vector<int> runs;
            int cur_run = 1;
            for (int i = 1; i < bit_width; ++i) {
                if (((r >> i) & 1) == ((r >> (i-1)) & 1)) {
                    cur_run++;
                } else {
                    runs.push_back(cur_run);
                    cur_run = 1;
                }
            }
            runs.push_back(cur_run);
            f.block_count = (int)runs.size();

            // Run-length entropy: treat each run's fraction of total bits as probability
            double rl_ent = 0.0;
            for (int rl : runs) {
                double p = (double)rl / bit_width;
                if (p > 0) rl_ent -= p * std::log2(p);
            }
            f.run_length_entropy = rl_ent;

            // Binary compressibility
            f.binary_compressibility = (f.block_count > 0)
                ? (double)bit_width / f.block_count : (double)bit_width;

            return f;
        };

        // Pre-compute static features per residue
        std::vector<ExtFeatures> static_feats(modulo);
        for (unsigned long long r = 0; r < modulo; ++r)
            static_feats[r] = extract_ext(r);

        // Simulation
        std::vector<unsigned long long> total_steps(modulo, 0);
        std::vector<unsigned long long> total_peak(modulo, 0);
        std::vector<unsigned long long> total_v2(modulo, 0);
        std::vector<unsigned long long> cnt(modulo, 0);
        std::vector<double> total_odd_mult(modulo, 0.0);

        auto t0 = std::chrono::high_resolution_clock::now();
        for (unsigned long long i = 1; i <= limit; i += 2) {
            CollatzFeatureEngine::SeqResult sr =
                CollatzFeatureEngine::run_sequence(i);
            unsigned long long r = i % modulo;
            total_steps[r] += sr.steps;
            total_peak[r]  += sr.peak;
            total_odd_mult[r] += sr.avg_odd_mult;
            cnt[r]++;
        }
        auto t1 = std::chrono::high_resolution_clock::now();
        long long sim_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

        // Build per-class vectors for correlation + regression
        std::vector<double> Y_steps, Y_peak, Y_v2;
        // 19 feature columns
        const int NF = 19;
        std::vector<std::vector<double>> FV(NF);
        const std::vector<std::string> F_NAMES = {
            "max_run_1s", "max_run_0s", "hamming_weight", "alternation_score",
            "trailing_ones", "trailing_zeros_3n1", "mod_3", "mod_9", "mod_27",
            "bit_entropy", "avg_odd_mult",
            "leading_ones", "leading_zeros", "trailing_zeros",
            "longest_alternating_run", "block_count",
            "run_length_entropy", "binary_compressibility", "max_run_0s_dup"
        };

        std::stringstream csv;
        csv << "residue";
        for (const auto& n : F_NAMES) csv << "," << n;
        csv << ",count,avg_steps,avg_peak,avg_v2\n";

        for (unsigned long long r = 0; r < modulo; ++r) {
            if (cnt[r] == 0) continue;
            double avg_steps = (double)total_steps[r] / cnt[r];
            double avg_peak  = (double)total_peak[r]  / cnt[r];
            double avg_v2    = (double)total_v2[r]    / cnt[r];
            double aom       = total_odd_mult[r] / cnt[r];
            static_feats[r].avg_odd_mult = aom;

            Y_steps.push_back(avg_steps);
            Y_peak.push_back(avg_peak);
            Y_v2.push_back(avg_v2);

            const ExtFeatures& ef = static_feats[r];
            std::vector<double> row = {
                (double)ef.max_run_1s, (double)ef.max_run_0s,
                (double)ef.hamming_weight, (double)ef.alternation_score,
                (double)ef.trailing_ones, (double)ef.trailing_zeros_3n1,
                (double)ef.mod_3, (double)ef.mod_9, (double)ef.mod_27,
                ef.bit_entropy, aom,
                (double)ef.leading_ones, (double)ef.leading_zeros,
                (double)ef.trailing_zeros,
                (double)ef.longest_alternating_run, (double)ef.block_count,
                ef.run_length_entropy, ef.binary_compressibility,
                (double)ef.max_run_0s_dup
            };
            for (int j = 0; j < NF; ++j) FV[j].push_back(row[j]);

            csv << r;
            for (double v : row) csv << "," << std::fixed << std::setprecision(4) << v;
            csv << "," << cnt[r] << ","
                << std::setprecision(4) << avg_steps << ","
                << avg_peak << "," << avg_v2 << "\n";
        }

        // ---- Full Pearson correlation matrix ----
        std::cout << "\n--- Pearson Correlation (feature vs avg_steps) ---\n";
        for (int j = 0; j < NF; ++j) {
            double r = CollatzFeatureEngine::pearson(FV[j], Y_steps);
            std::cout << "  " << std::left << std::setw(26) << F_NAMES[j]
                      << ": " << std::fixed << std::setprecision(4) << r << "\n";
        }

        // ---- 19-feature OLS ----
        RegressionModel m =
            CollatzFeatureEngine::fit_ols(FV, Y_steps, F_NAMES);

        std::cout << "\n--- 19-Feature Multiple Regression ---\n";
        std::cout << "Joint R² : " << std::fixed << std::setprecision(4)
                  << m.r_squared << "\n";
        std::cout << "Intercept: " << m.intercept << "\n";
        std::cout << "Weights:\n";
        for (int j = 0; j < NF; ++j)
            std::cout << "  " << std::left << std::setw(26) << F_NAMES[j]
                      << ": " << m.weights[j] << "\n";

        std::cout << "\n  Simulation time : " << sim_ms << " ms\n";

        DataExporter::export_csv(
            "advanced_binary_mod_" + std::to_string(modulo)
            + "_limit_" + std::to_string(limit) + ".csv",
            csv.str());
    }
};

} // namespace research
} // namespace collatz
