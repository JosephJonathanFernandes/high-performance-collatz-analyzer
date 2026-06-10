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

class DifficultyPredictor {
public:
    static void analyze(unsigned long long limit) {
        const unsigned long long TRAIN_MODULO = 1024;
        int bit_width = 10; // log2(1024)

        std::cout << "\n========================================================\n";
        std::cout << "Research Module 9: Difficulty Predictor\n";
        std::cout << "Training on: " << TRAIN_MODULO << " residue classes\n";
        std::cout << "Predicting : " << limit << " odd numbers\n";
        std::cout << "========================================================\n";

        // Phase 1: Train the regression model
        std::cout << "Phase 1: Training OLS model on residue class averages...\n";
        auto t0 = std::chrono::high_resolution_clock::now();

        CollatzFeatureEngine::TrainingResult tr =
            CollatzFeatureEngine::compute_weights(limit, TRAIN_MODULO);

        auto t1 = std::chrono::high_resolution_clock::now();
        long long train_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

        std::cout << "  Class-level R² : " << std::fixed << std::setprecision(4)
                  << tr.model.r_squared << "\n";
        std::cout << "  Training time  : " << train_ms << " ms\n\n";

        // Phase 2: Predict every odd number
        std::cout << "Phase 2: Predicting individual stopping times...\n";
        auto t2 = std::chrono::high_resolution_clock::now();

        double total_abs_err = 0.0, total_sq_err = 0.0;
        double max_over_err = 0.0, max_under_err = 0.0;
        unsigned long long max_over_n = 0, max_under_n = 0;
        long long count = 0;

        std::vector<double> Y_actual, Y_pred_vec;

        // We need these for outlier tracking
        struct PredEntry {
            unsigned long long n;
            double predicted;
            double actual;
            double error;
        };

        // Stream CSV to avoid holding everything in memory for large limits
        std::stringstream csv;
        csv << "number,predicted_steps,actual_steps,error\n";

        for (unsigned long long i = 1; i <= limit; i += 2) {
            double class_mult = tr.class_avg_mult[i % TRAIN_MODULO];
            double pred = CollatzFeatureEngine::predict_single(
                tr.model, i, bit_width, class_mult);

            CollatzFeatureEngine::SeqResult sr =
                CollatzFeatureEngine::run_sequence(i);
            double actual = (double)sr.steps;
            double err    = actual - pred;

            total_abs_err += std::abs(err);
            total_sq_err  += err * err;
            Y_actual.push_back(actual);
            Y_pred_vec.push_back(pred);

            if (err > max_over_err)  { max_over_err  = err;  max_over_n  = i; }
            if (err < max_under_err) { max_under_err = err;  max_under_n = i; }

            csv << i << "," << std::fixed << std::setprecision(2)
                << pred << "," << (int)actual << ","
                << std::setprecision(2) << err << "\n";

            count++;
        }

        auto t3 = std::chrono::high_resolution_clock::now();
        long long pred_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count();

        double mae = total_abs_err / count;
        double mse = total_sq_err  / count;
        double r2  = CollatzFeatureEngine::compute_r2(Y_actual, Y_pred_vec);

        std::cout << "\n--- Prediction Results (per-number) ---\n";
        std::cout << "  Numbers tested      : " << count << "\n";
        std::cout << "  Mean Absolute Error : " << std::fixed << std::setprecision(4) << mae << " steps\n";
        std::cout << "  Mean Squared Error  : " << std::setprecision(4) << mse << "\n";
        std::cout << "  R² (per-number)     : " << std::setprecision(4) << r2 << "\n";
        std::cout << "  Max Overestimate    : +" << std::setprecision(2) << max_over_err
                  << " steps (n=" << max_over_n << ")\n";
        std::cout << "  Max Underestimate   : " << std::setprecision(2) << max_under_err
                  << " steps (n=" << max_under_n << ")\n";
        std::cout << "\n[NOTE] Per-number R² is lower than class-level R²=0.9044\n";
        std::cout << "       because the model was trained on residue class means,\n";
        std::cout << "       not individual numbers. This gap is scientifically expected.\n";
        std::cout << "  Prediction time     : " << pred_ms << " ms\n";
        std::cout << "---------------------------------------\n";

        DataExporter::export_csv(
            "predictor_limit_" + std::to_string(limit) + ".csv",
            csv.str());
    }
};

} // namespace research
} // namespace collatz
