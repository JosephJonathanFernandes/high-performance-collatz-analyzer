#pragma once

#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <string>
#include <numeric>
#include "CollatzFeatureEngine.hpp"
#include "DataExporter.hpp"

namespace collatz {
namespace research {

class FeatureImportanceAnalyzer {
public:
    static void analyze(unsigned long long limit) {
        const unsigned long long TRAIN_MODULO = 1024;

        std::cout << "\n========================================================\n";
        std::cout << "Research Module 12: Feature Importance Analyzer\n";
        std::cout << "Limit  : " << limit << "\n";
        std::cout << "Methods: Leave-One-Out (LOO)  +  Permutation Importance\n";
        std::cout << "========================================================\n";

        // Phase 1: Train baseline 11-feature model
        std::cout << "Training baseline model...\n";
        auto t0 = std::chrono::high_resolution_clock::now();
        CollatzFeatureEngine::TrainingResult tr =
            CollatzFeatureEngine::compute_weights(limit, TRAIN_MODULO);
        auto t1 = std::chrono::high_resolution_clock::now();
        double baseline_r2 = tr.model.r_squared;
        std::cout << "  Baseline R² : " << std::fixed << std::setprecision(4) << baseline_r2 << "\n";
        std::cout << "  Time        : "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()
                  << " ms\n\n";

        // Re-collect the feature vectors used during training so we can LOO retrain
        // We rebuild them from the class averages already computed
        int bit_width = 10;
        struct ClassRow {
            double avg_steps;
            std::vector<double> features; // 11 features in order
        };

        std::vector<ClassRow> rows;
        for (unsigned long long r = 0; r < TRAIN_MODULO; ++r) {
            // Reconstruct features from extract_features
            NumberFeatures f =
                CollatzFeatureEngine::extract_features(r, bit_width);
            double aom = tr.class_avg_mult[r];
            f.avg_odd_mult = aom;

            // We need avg_steps — re-run a tiny simulation? No.
            // Instead, use the model to predict — that's circular.
            // Instead, store placeholder. We'll collect from simulation.
            // NOTE: We do a secondary lightweight pass collecting (r, avg_steps).
        }

        // Lightweight second pass: collect avg_steps per class (using stored totals)
        // Re-run simulation briefly to collect class stats
        std::vector<unsigned long long> class_total_steps(TRAIN_MODULO, 0);
        std::vector<unsigned long long> class_count(TRAIN_MODULO, 0);

        for (unsigned long long i = 1; i <= limit; i += 2) {
            CollatzFeatureEngine::SeqResult sr =
                CollatzFeatureEngine::run_sequence(i);
            unsigned long long r = i % TRAIN_MODULO;
            class_total_steps[r] += sr.steps;
            class_count[r]++;
        }

        std::vector<double> Y_vec;
        // 11 feature columns
        std::vector<std::vector<double>> X_all(11);
        const std::vector<std::string>& feat_names = tr.model.feature_names;

        for (unsigned long long r = 0; r < TRAIN_MODULO; ++r) {
            if (class_count[r] == 0) continue;
            double avg_steps = (double)class_total_steps[r] / class_count[r];
            Y_vec.push_back(avg_steps);

            NumberFeatures f =
                CollatzFeatureEngine::extract_features(r, bit_width);
            double aom = tr.class_avg_mult[r];

            X_all[0].push_back(f.max_run_1s);
            X_all[1].push_back(f.alternation_score);
            X_all[2].push_back(f.hamming_weight);
            X_all[3].push_back(0.0); // v2 placeholder
            X_all[4].push_back(f.trailing_ones);
            X_all[5].push_back(f.trailing_zeros_3n1);
            X_all[6].push_back(f.mod_3);
            X_all[7].push_back(f.mod_9);
            X_all[8].push_back(f.mod_27);
            X_all[9].push_back(f.bit_entropy);
            X_all[10].push_back(aom);
        }

        int K = (int)X_all.size();
        int N = (int)Y_vec.size();

        // ---- Leave-One-Out importance ----
        std::cout << "Running Leave-One-Out analysis...\n";
        std::vector<double> loo_delta(K);

        for (int j = 0; j < K; ++j) {
            std::vector<std::vector<double>> X_loo;
            std::vector<std::string> names_loo;
            for (int k = 0; k < K; ++k) {
                if (k == j) continue;
                X_loo.push_back(X_all[k]);
                names_loo.push_back(feat_names[k]);
            }
            RegressionModel m =
                CollatzFeatureEngine::fit_ols(X_loo, Y_vec, names_loo);
            loo_delta[j] = baseline_r2 - m.r_squared;
        }

        // ---- Permutation importance ----
        std::cout << "Running Permutation Importance analysis...\n";
        std::vector<double> perm_delta(K);

        // Use baseline fitted values as reference
        std::vector<double> baseline_pred(N);
        for (int i = 0; i < N; ++i) {
            baseline_pred[i] = tr.model.intercept;
            for (int j = 0; j < K; ++j)
                baseline_pred[i] += tr.model.weights[j] * X_all[j][i];
        }
        double r2_baseline_check = CollatzFeatureEngine::compute_r2(Y_vec, baseline_pred);

        // Simple deterministic shuffle using index rotation (reproducible, no <random>)
        auto shuffle_vec = [](std::vector<double> v, int seed) {
            int n = (int)v.size();
            for (int i = n - 1; i > 0; --i) {
                int j = (i * 6271 + seed * 1009 + 31337) % (i + 1);
                if (j < 0) j = -j;
                std::swap(v[i], v[j]);
            }
            return v;
        };

        for (int j = 0; j < K; ++j) {
            std::vector<double> X_shuffled = shuffle_vec(X_all[j], 42 + j);
            std::vector<double> perm_pred(N);
            for (int i = 0; i < N; ++i) {
                perm_pred[i] = tr.model.intercept;
                for (int k = 0; k < K; ++k) {
                    double val = (k == j) ? X_shuffled[i] : X_all[k][i];
                    perm_pred[i] += tr.model.weights[k] * val;
                }
            }
            double r2_perm = CollatzFeatureEngine::compute_r2(Y_vec, perm_pred);
            perm_delta[j] = r2_baseline_check - r2_perm;
        }

        // ---- Sort by LOO importance ----
        std::vector<int> rank_idx(K);
        std::iota(rank_idx.begin(), rank_idx.end(), 0);
        std::sort(rank_idx.begin(), rank_idx.end(),
                  [&](int a, int b){ return loo_delta[a] > loo_delta[b]; });

        std::cout << "\n--- Feature Importance Report ---\n";
        std::cout << "Baseline R² = " << std::fixed << std::setprecision(4) << baseline_r2 << "\n\n";
        std::cout << std::left
                  << std::setw(4) << "Rank"
                  << std::setw(26) << "Feature"
                  << std::setw(14) << "LOO ΔR²"
                  << std::setw(18) << "Permutation ΔR²"
                  << "\n";
        std::cout << std::string(62, '-') << "\n";

        std::stringstream csv;
        csv << "rank,feature,loo_delta_r2,permutation_delta_r2\n";

        for (int rank = 0; rank < K; ++rank) {
            int j = rank_idx[rank];
            std::cout << std::left
                      << std::setw(4) << (rank + 1)
                      << std::setw(26) << feat_names[j]
                      << std::setw(14) << std::setprecision(4) << loo_delta[j]
                      << std::setprecision(4) << perm_delta[j] << "\n";
            csv << (rank + 1) << ","
                << feat_names[j] << ","
                << std::fixed << std::setprecision(6) << loo_delta[j] << ","
                << std::setprecision(6) << perm_delta[j] << "\n";
        }
        std::cout << std::string(62, '-') << "\n";

        DataExporter::export_csv(
            "feature_importance_limit_" + std::to_string(limit) + ".csv",
            csv.str());
    }
};

} // namespace research
} // namespace collatz
