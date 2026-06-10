#pragma once

#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cmath>
#include <algorithm>
#include "CollatzFeatureEngine.hpp"
#include "DataExporter.hpp"

namespace collatz {
namespace research {

// ============================================================
// Module 16: Odd-to-Odd Drift Spectrum Analyzer
//
// For each residue class mod <modulo>, computes:
//   avg_v2        : E[v_2(3n+1)]
//   avg_log_drift : E[log(T(n)/n)] = E[log(3) - v_2 * log(2)]
//   avg_odd_mult  : E[3 / 2^v_2]
//
// Then measures the Pearson correlation between each of these
// drift metrics and the class's average stopping time.
//
// Hypothesis:
//   Correlation(avg_stopping_time, avg_log_drift) > 0.95
// ============================================================

class OddToOddDriftSpectrum {
public:
    static void analyze(unsigned long long limit,
                        unsigned long long modulo = 1024)
    {
        if ((modulo & (modulo - 1)) != 0 || modulo == 0) {
            std::cerr << "[ERROR] Modulo must be a power of 2.\n";
            return;
        }

        std::cout << "\n========================================================\n";
        std::cout << "Research Module 16: Odd-to-Odd Drift Spectrum Analyzer\n";
        std::cout << "Modulo : " << modulo << "  Limit : " << limit << "\n";
        std::cout << "========================================================\n";
        std::cout << "Hypothesis: Correlation(avg_steps, E[log(T(n)/n)]) > 0.95\n\n";

        const double LOG2  = std::log(2.0);
        const double LOG3  = std::log(3.0);

        struct ClassStats {
            unsigned long long count         = 0;
            unsigned long long total_steps   = 0;
            double total_log_drift           = 0.0; // sum of per-number E[log(T/n)]
            double total_avg_v2              = 0.0; // sum of per-number E[v2]
            double total_avg_odd_mult        = 0.0; // sum of per-number E[3/2^v2]
            unsigned long long total_odd_steps = 0; // total odd-to-odd applications
        };

        std::vector<ClassStats> stats(modulo);

        auto t0 = std::chrono::high_resolution_clock::now();

        for (unsigned long long i = 1; i <= limit; i += 2) {
            unsigned long long cur = i;
            int steps = 0;
            double sum_log_drift = 0.0;
            double sum_v2        = 0.0;
            double sum_mult      = 0.0;
            int    odd_steps     = 0;

            while (cur != 1) {
                if (cur & 1) {
                    // One odd-to-odd application: T(cur) = (3*cur+1) / 2^v2
                    cur = 3 * cur + 1;
                    int v2 = 0;
                    while ((cur & 1) == 0) { cur >>= 1; steps++; v2++; }

                    // log(T(n)/n) = log(3) - v2*log(2)  (approximation ignoring +1)
                    double log_drift = LOG3 - v2 * LOG2;
                    sum_log_drift += log_drift;
                    sum_v2        += v2;
                    sum_mult      += (v2 < 62) ? 3.0 / (1ULL << v2) : 0.0;
                    odd_steps++;
                } else {
                    cur >>= 1;
                    steps++;
                }
            }

            unsigned long long r = i % modulo;
            stats[r].count++;
            stats[r].total_steps += steps;
            if (odd_steps > 0) {
                stats[r].total_log_drift    += sum_log_drift / odd_steps;
                stats[r].total_avg_v2       += sum_v2 / odd_steps;
                stats[r].total_avg_odd_mult += sum_mult / odd_steps;
            }
            stats[r].total_odd_steps += odd_steps;
        }

        auto t1 = std::chrono::high_resolution_clock::now();
        long long sim_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

        // Build per-class vectors
        std::vector<double> Y_steps;
        std::vector<double> avg_log_drift_vec;
        std::vector<double> avg_v2_vec;
        std::vector<double> avg_odd_mult_vec;

        std::stringstream csv;
        csv << "residue,count,avg_steps,avg_v2,avg_log_drift,avg_odd_mult,"
               "theoretical_drift\n";

        // Theoretical E[log drift] = sum_{k>=1} P(v2=k) * (log3 - k*log2)
        // Under the uniform bit model: P(v2=k) = 2^{-k}, so:
        // E[log drift] = log3 - log2 * sum_{k>=1} k * 2^{-k}
        //              = log3 - log2 * 2   (since sum k*2^{-k} = 2)
        //              = log3 - 2*log2 = log(3/4) = -0.287682
        const double THEORETICAL_DRIFT = LOG3 - 2.0 * LOG2; // = ln(3/4)

        // Track extremes for console summary
        double max_steps_val = 0, min_steps_val = 1e18;
        unsigned long long hard_r = 0, easy_r = 0;

        for (unsigned long long r = 0; r < modulo; ++r) {
            if (stats[r].count == 0) continue;
            double avg_steps = (double)stats[r].total_steps / stats[r].count;
            double avg_v2    = stats[r].total_avg_v2       / stats[r].count;
            double avg_drift = stats[r].total_log_drift    / stats[r].count;
            double avg_mult  = stats[r].total_avg_odd_mult / stats[r].count;

            Y_steps.push_back(avg_steps);
            avg_log_drift_vec.push_back(avg_drift);
            avg_v2_vec.push_back(avg_v2);
            avg_odd_mult_vec.push_back(avg_mult);

            csv << r << "," << stats[r].count << ","
                << std::fixed << std::setprecision(4)
                << avg_steps << "," << avg_v2 << ","
                << avg_drift << "," << avg_mult << ","
                << THEORETICAL_DRIFT << "\n";

            if (avg_steps > max_steps_val) { max_steps_val = avg_steps; hard_r = r; }
            if (avg_steps < min_steps_val) { min_steps_val = avg_steps; easy_r = r; }
        }

        // Pearson correlations
        double r_drift = CollatzFeatureEngine::pearson(Y_steps, avg_log_drift_vec);
        double r_v2    = CollatzFeatureEngine::pearson(Y_steps, avg_v2_vec);
        double r_mult  = CollatzFeatureEngine::pearson(Y_steps, avg_odd_mult_vec);

        // Observed mean drift
        double sum_drift = 0.0;
        for (double d : avg_log_drift_vec) sum_drift += d;
        double mean_observed_drift = sum_drift / avg_log_drift_vec.size();

        // ---- Console output ----
        std::cout << "--- Drift Spectrum Results ---\n\n";
        std::cout << "  Theoretical E[log(T(n)/n)] = ln(3/4) = "
                  << std::fixed << std::setprecision(6) << THEORETICAL_DRIFT << "\n";
        std::cout << "  Observed mean drift (class avg): "
                  << std::setprecision(6) << mean_observed_drift << "\n\n";

        std::cout << "  Hardest residue : " << hard_r
                  << "  avg_steps=" << std::setprecision(2) << max_steps_val << "\n";
        std::cout << "  Easiest residue : " << easy_r
                  << "  avg_steps=" << std::setprecision(2) << min_steps_val << "\n\n";

        std::cout << "--- Correlation with avg_steps ---\n";
        std::cout << "  Corr(avg_steps, avg_log_drift) : "
                  << std::setprecision(4) << r_drift << "\n";
        std::cout << "  Corr(avg_steps, avg_v2)        : "
                  << std::setprecision(4) << r_v2 << "\n";
        std::cout << "  Corr(avg_steps, avg_odd_mult)  : "
                  << std::setprecision(4) << r_mult << "\n\n";

        // Interpret the result relative to the hypothesis
        std::cout << "--- Hypothesis Test ---\n";
        std::cout << "  H: Correlation(avg_steps, avg_log_drift) > 0.95\n";
        double abs_drift_corr = std::abs(r_drift);
        if (abs_drift_corr > 0.95)
            std::cout << "  RESULT: SUPPORTED  (|r| = " << std::setprecision(4)
                      << abs_drift_corr << " > 0.95)\n";
        else if (abs_drift_corr > 0.85)
            std::cout << "  RESULT: PARTIALLY SUPPORTED  (|r| = " << std::setprecision(4)
                      << abs_drift_corr << ", strong but below 0.95)\n";
        else
            std::cout << "  RESULT: NOT SUPPORTED  (|r| = " << std::setprecision(4)
                      << abs_drift_corr << ")\n";

        std::cout << "\n--- Scientific Interpretation ---\n";
        std::cout << "  avg_log_drift per class is E[log(3/2^v2)] averaged over\n";
        std::cout << "  the entire trajectory. A class with a more NEGATIVE drift\n";
        std::cout << "  converges FASTER — higher v2 means more divisions per step.\n";
        std::cout << "  The binary features (run lengths, entropy) matter primarily\n";
        std::cout << "  because they influence v2, which determines the drift.\n\n";

        // R² of a single-feature regression: avg_steps ~ avg_log_drift
        // R² = r^2 for simple linear regression
        double r2_single = r_drift * r_drift;
        double r2_mult_single = r_mult * r_mult;
        std::cout << "  R² of avg_steps ~ avg_log_drift  (1 feature): "
                  << std::setprecision(4) << r2_single << "\n";
        std::cout << "  R² of avg_steps ~ avg_odd_mult   (1 feature): "
                  << std::setprecision(4) << r2_mult_single << "\n";
        std::cout << "  Compare: 11-feature binary model R² = 0.9044\n\n";

        std::cout << "  Simulation time : " << sim_ms << " ms\n";

        DataExporter::export_csv(
            "drift_spectrum_mod_" + std::to_string(modulo)
            + "_limit_" + std::to_string(limit) + ".csv",
            csv.str());
    }
};

} // namespace research
} // namespace collatz
