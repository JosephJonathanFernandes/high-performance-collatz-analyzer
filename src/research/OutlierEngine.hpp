#pragma once

#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <bitset>
#include "CollatzFeatureEngine.hpp"
#include "DataExporter.hpp"

namespace collatz {
namespace research {

class OutlierEngine {
public:
    static void analyze(unsigned long long limit, int top_k = 100) {
        const unsigned long long TRAIN_MODULO = 1024;
        const int BIT_WIDTH = 10;

        std::cout << "\n========================================================\n";
        std::cout << "Research Module 10: Outlier Discovery Engine\n";
        std::cout << "Limit   : " << limit << "  |  Top-K : " << top_k << "\n";
        std::cout << "========================================================\n";

        // Train model
        std::cout << "Training model...\n";
        auto t0 = std::chrono::high_resolution_clock::now();
        CollatzFeatureEngine::TrainingResult tr =
            CollatzFeatureEngine::compute_weights(limit, TRAIN_MODULO);
        auto t1 = std::chrono::high_resolution_clock::now();
        std::cout << "  Class-level R² : " << std::fixed << std::setprecision(4)
                  << tr.model.r_squared << "\n";
        std::cout << "  Training time  : "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()
                  << " ms\n";

        struct Entry {
            unsigned long long n;
            double predicted;
            double actual;
            double error;
            unsigned long long peak;
        };

        // Use partial collections — keep only top_k in each direction
        // to avoid storing all entries in RAM for huge limits
        std::vector<Entry> positives; // actual > predicted
        std::vector<Entry> negatives; // actual < predicted
        positives.reserve(top_k + 1);
        negatives.reserve(top_k + 1);

        auto insert_sorted = [&](std::vector<Entry>& vec, Entry e, bool max_first) {
            vec.push_back(e);
            // bubble new element into position
            for (int i = (int)vec.size() - 1; i > 0; --i) {
                bool swap = max_first ? (vec[i].error > vec[i-1].error)
                                      : (vec[i].error < vec[i-1].error);
                if (swap) std::swap(vec[i], vec[i-1]);
                else break;
            }
            if ((int)vec.size() > top_k) vec.pop_back();
        };

        std::cout << "Scanning outliers...\n";
        auto t2 = std::chrono::high_resolution_clock::now();

        for (unsigned long long i = 1; i <= limit; i += 2) {
            double class_mult = tr.class_avg_mult[i % TRAIN_MODULO];
            double pred = CollatzFeatureEngine::predict_single(
                tr.model, i, BIT_WIDTH, class_mult);
            CollatzFeatureEngine::SeqResult sr =
                CollatzFeatureEngine::run_sequence(i);
            double actual = (double)sr.steps;
            double err    = actual - pred;

            Entry e{i, pred, actual, err, sr.peak};
            if (err > 0) insert_sorted(positives, e, true);
            else         insert_sorted(negatives, e, false);
        }

        auto t3 = std::chrono::high_resolution_clock::now();
        long long scan_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count();

        auto print_group = [&](const std::vector<Entry>& group, const char* label) {
            std::cout << "\n--- " << label << " ---\n";
            std::cout << std::left
                      << std::setw(14) << "Number"
                      << std::setw(10) << "Actual"
                      << std::setw(10) << "Predicted"
                      << std::setw(10) << "Error"
                      << "Binary Pattern\n";
            std::cout << std::string(70, '-') << "\n";
            for (const auto& e : group) {
                // Build binary string (10 bits, LSB first reversed to MSB first)
                std::string bits(BIT_WIDTH, '0');
                for (int b = 0; b < BIT_WIDTH; ++b)
                    bits[BIT_WIDTH - 1 - b] = ((e.n >> b) & 1) ? '1' : '0';
                std::cout << std::left
                          << std::setw(14) << e.n
                          << std::setw(10) << (int)e.actual
                          << std::setw(10) << std::fixed << std::setprecision(1) << e.predicted
                          << std::setw(10) << std::setprecision(1) << e.error
                          << bits << "\n";
            }
        };

        print_group(positives, "Top Hardest Outliers (Model Underestimates)");
        print_group(negatives, "Top Easiest Outliers (Model Overestimates)");

        std::cout << "\n  Scan time : " << scan_ms << " ms\n";

        // Build CSV
        std::stringstream csv;
        csv << "number,predicted_steps,actual_steps,error,peak_value,binary_pattern\n";
        auto emit = [&](const std::vector<Entry>& group) {
            for (const auto& e : group) {
                std::string bits(BIT_WIDTH, '0');
                for (int b = 0; b < BIT_WIDTH; ++b)
                    bits[BIT_WIDTH - 1 - b] = ((e.n >> b) & 1) ? '1' : '0';
                csv << e.n << ","
                    << std::fixed << std::setprecision(2) << e.predicted << ","
                    << (int)e.actual << ","
                    << std::setprecision(2) << e.error << ","
                    << e.peak << ","
                    << bits << "\n";
            }
        };
        emit(positives);
        emit(negatives);

        DataExporter::export_csv(
            "outliers_limit_" + std::to_string(limit) + ".csv",
            csv.str());
    }
};

} // namespace research
} // namespace collatz
