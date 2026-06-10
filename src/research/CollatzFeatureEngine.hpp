#pragma once

#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

namespace collatz {
namespace research {

// ============================================================
// NumberFeatures: all 11 arithmetic/binary features for n
// ============================================================
struct NumberFeatures {
    // Binary structure (computed from n's bit representation)
    int max_run_1s          = 0;
    int max_run_0s          = 0;
    int hamming_weight      = 0;
    int alternation_score   = 0;
    int trailing_ones       = 0;
    int trailing_zeros_3n1  = 0;
    double bit_entropy      = 0.0;

    // Ternary modular bounds
    int mod_3   = 0;
    int mod_9   = 0;
    int mod_27  = 0;

    // Geometric — filled by simulation (set to residue-class average during training)
    double avg_odd_mult = 0.0;
};

// ============================================================
// RegressionModel: trained OLS weights
// ============================================================
struct RegressionModel {
    double intercept = 0.0;
    std::vector<double> weights;
    std::vector<std::string> feature_names;
    double r_squared = 0.0;
    bool trained = false;

    double predict(const NumberFeatures& f) const {
        if (!trained) return 0.0;
        std::vector<double> fv = to_vec(f);
        double val = intercept;
        for (int i = 0; i < (int)weights.size(); ++i)
            val += weights[i] * fv[i];
        return val;
    }

    static std::vector<double> to_vec(const NumberFeatures& f) {
        return {
            (double)f.max_run_1s,
            (double)f.alternation_score,
            (double)f.hamming_weight,
            f.avg_odd_mult,
            (double)f.trailing_ones,
            (double)f.trailing_zeros_3n1,
            (double)f.mod_3,
            (double)f.mod_9,
            (double)f.mod_27,
            f.bit_entropy,
            f.avg_odd_mult   // avg_odd_mult listed twice intentionally — first
                             // slot is per-number, second is class avg during predict
        };
    }
};

// ============================================================
// CollatzFeatureEngine
// ============================================================
class CollatzFeatureEngine {
public:
    // Extract static binary/arithmetic features for a single number n.
    // avg_odd_mult is NOT filled here — set it from the residue-class average.
    static NumberFeatures extract_features(unsigned long long n, int bit_width) {
        NumberFeatures f;
        f.mod_3  = n % 3;
        f.mod_9  = n % 9;
        f.mod_27 = n % 27;

        int current_run_1 = 0, max_run_1 = 0;
        int current_run_0 = 0, max_run_0 = 0;
        int weight = 0, alternations = 0, last_bit = -1;
        int trailing_1s = 0;
        bool still_trailing = true;

        for (int i = 0; i < bit_width; ++i) {
            int bit = (n >> i) & 1;
            if (bit == 1) {
                weight++;
                current_run_1++;
                if (current_run_1 > max_run_1) max_run_1 = current_run_1;
                current_run_0 = 0;
                if (still_trailing) trailing_1s++;
            } else {
                current_run_0++;
                if (current_run_0 > max_run_0) max_run_0 = current_run_0;
                current_run_1 = 0;
                still_trailing = false;
            }
            if (last_bit != -1 && bit != last_bit) alternations++;
            last_bit = bit;
        }

        f.max_run_1s        = max_run_1;
        f.max_run_0s        = max_run_0;
        f.hamming_weight    = weight;
        f.alternation_score = alternations;
        f.trailing_ones     = trailing_1s;

        unsigned long long r_next = 3 * n + 1;
        int tz = 0;
        while ((r_next & 1) == 0 && r_next > 0) { tz++; r_next >>= 1; }
        f.trailing_zeros_3n1 = tz;

        double p1 = (bit_width > 0) ? (double)weight / bit_width : 0.0;
        double p0 = 1.0 - p1;
        double entropy = 0.0;
        if (p1 > 0) entropy -= p1 * std::log2(p1);
        if (p0 > 0) entropy -= p0 * std::log2(p0);
        f.bit_entropy = entropy;

        return f;
    }

    // Run actual Collatz sequence, return {steps, peak, avg_odd_mult}
    struct SeqResult {
        int steps = 0;
        unsigned long long peak = 0;
        double avg_odd_mult = 0.0;
    };

    static SeqResult run_sequence(unsigned long long n) {
        SeqResult r;
        r.peak = n;
        unsigned long long cur = n;
        double total_mult = 0.0;
        int mult_count = 0;

        while (cur != 1) {
            if (cur > r.peak) r.peak = cur;
            if (cur & 1) {
                cur = 3 * cur + 1;
                int v2 = 0;
                while ((cur & 1) == 0) { cur >>= 1; r.steps++; v2++; }
                if (v2 > 0) {
                    total_mult += 3.0 / (1ULL << (v2 < 62 ? v2 : 62));
                    mult_count++;
                }
            } else {
                cur >>= 1;
                r.steps++;
            }
        }
        r.avg_odd_mult = (mult_count > 0) ? total_mult / mult_count : 0.0;
        return r;
    }

    // --------------------------------------------------------
    // OLS solver (Gaussian elimination with partial pivoting)
    // --------------------------------------------------------
    static std::vector<double> solve_ols(
        std::vector<std::vector<double>> A,
        std::vector<double> B)
    {
        int n = (int)B.size();
        for (int i = 0; i < n; i++) {
            int maxRow = i;
            for (int k = i + 1; k < n; k++)
                if (std::abs(A[k][i]) > std::abs(A[maxRow][i])) maxRow = k;
            std::swap(A[i], A[maxRow]);
            std::swap(B[i], B[maxRow]);
            if (std::abs(A[i][i]) < 1e-12) continue;
            for (int k = i + 1; k < n; k++) {
                double c = A[k][i] / A[i][i];
                for (int j = i; j < n; j++) A[k][j] -= c * A[i][j];
                B[k] -= c * B[i];
            }
        }
        std::vector<double> x(n, 0.0);
        for (int i = n - 1; i >= 0; i--) {
            if (std::abs(A[i][i]) < 1e-12) continue;
            x[i] = B[i];
            for (int j = i + 1; j < n; j++) x[i] -= A[i][j] * x[j];
            x[i] /= A[i][i];
        }
        return x;
    }

    static double compute_r2(
        const std::vector<double>& Y,
        const std::vector<double>& Y_pred)
    {
        double sum_y = 0;
        for (double y : Y) sum_y += y;
        double mean_y = sum_y / Y.size();
        double ss_tot = 0, ss_res = 0;
        for (int i = 0; i < (int)Y.size(); ++i) {
            ss_tot += (Y[i] - mean_y) * (Y[i] - mean_y);
            ss_res += (Y[i] - Y_pred[i]) * (Y[i] - Y_pred[i]);
        }
        return (ss_tot < 1e-12) ? 1.0 : 1.0 - ss_res / ss_tot;
    }

    // --------------------------------------------------------
    // fit_ols: fit model from feature matrix and target vector
    // --------------------------------------------------------
    static RegressionModel fit_ols(
        const std::vector<std::vector<double>>& X_cols,
        const std::vector<double>& Y,
        const std::vector<std::string>& feature_names)
    {
        int n = (int)Y.size();
        int k = (int)X_cols.size();
        int p = k + 1;

        // Build design matrix X (n x p) with intercept column
        std::vector<std::vector<double>> X(n, std::vector<double>(p, 1.0));
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < k; ++j)
                X[i][j + 1] = X_cols[j][i];

        // Form XtX (p x p) and XtY (p)
        std::vector<std::vector<double>> XtX(p, std::vector<double>(p, 0.0));
        std::vector<double> XtY(p, 0.0);
        for (int i = 0; i < p; ++i) {
            for (int j = 0; j < p; ++j)
                for (int r = 0; r < n; ++r)
                    XtX[i][j] += X[r][i] * X[r][j];
            for (int r = 0; r < n; ++r)
                XtY[i] += X[r][i] * Y[r];
        }

        std::vector<double> w = solve_ols(XtX, XtY);

        // Compute fitted values and R²
        std::vector<double> Y_pred(n);
        for (int i = 0; i < n; ++i) {
            Y_pred[i] = w[0];
            for (int j = 0; j < k; ++j)
                Y_pred[i] += w[j + 1] * X_cols[j][i];
        }

        RegressionModel model;
        model.intercept     = w[0];
        model.weights       = std::vector<double>(w.begin() + 1, w.end());
        model.feature_names = feature_names;
        model.r_squared     = compute_r2(Y, Y_pred);
        model.trained       = true;
        return model;
    }

    // --------------------------------------------------------
    // compute_weights: train the 11-feature model from scratch.
    // Runs the residue-class simulation over [1, limit] odd numbers,
    // returns fitted model + per-class avg_odd_mult lookup table.
    // --------------------------------------------------------
    struct TrainingResult {
        RegressionModel model;
        // avg_odd_mult for each residue class mod 1024
        std::vector<double> class_avg_mult;
        unsigned long long modulo = 1024;
    };

    static TrainingResult compute_weights(unsigned long long limit,
                                          unsigned long long modulo = 1024)
    {
        // modulo must be power of 2
        int bit_width = 0;
        { unsigned long long t = modulo; while (t > 1) { t >>= 1; bit_width++; } }

        struct ClassStats {
            unsigned long long count = 0;
            unsigned long long total_steps = 0;
            unsigned long long total_v2 = 0;
            unsigned long long total_peak = 0;
            double total_odd_mult = 0.0;
            int max_run_1s = 0, max_run_0s = 0;
            int hamming_weight = 0, alternation_score = 0;
            int trailing_ones = 0, trailing_zeros_3n1 = 0;
            int mod_3 = 0, mod_9 = 0, mod_27 = 0;
            double bit_entropy = 0.0;
        };

        std::vector<ClassStats> stats(modulo);

        // Pre-compute static features per residue
        for (unsigned long long r = 0; r < modulo; ++r) {
            NumberFeatures f = extract_features(r, bit_width);
            stats[r].max_run_1s          = f.max_run_1s;
            stats[r].max_run_0s          = f.max_run_0s;
            stats[r].hamming_weight      = f.hamming_weight;
            stats[r].alternation_score   = f.alternation_score;
            stats[r].trailing_ones       = f.trailing_ones;
            stats[r].trailing_zeros_3n1  = f.trailing_zeros_3n1;
            stats[r].mod_3               = f.mod_3;
            stats[r].mod_9               = f.mod_9;
            stats[r].mod_27              = f.mod_27;
            stats[r].bit_entropy         = f.bit_entropy;
        }

        // Simulation loop
        for (unsigned long long i = 1; i <= limit; i += 2) {
            SeqResult sr = run_sequence(i);
            unsigned long long r = i % modulo;
            stats[r].count++;
            stats[r].total_steps    += sr.steps;
            stats[r].total_peak     += sr.peak;
            stats[r].total_odd_mult += sr.avg_odd_mult;
        }

        // Build feature vectors
        std::vector<double> Y;
        std::vector<double> run1s, alts, ham, v2_dummy, trail1, tz3n1;
        std::vector<double> m3, m9, m27, ent, aom;
        std::vector<double> avg_mult_per_class(modulo, 0.0);

        for (unsigned long long r = 0; r < modulo; ++r) {
            if (stats[r].count == 0) continue;
            double avg_steps = (double)stats[r].total_steps / stats[r].count;
            double avg_om    = stats[r].total_odd_mult / stats[r].count;
            avg_mult_per_class[r] = avg_om;

            Y.push_back(avg_steps);
            run1s .push_back(stats[r].max_run_1s);
            alts  .push_back(stats[r].alternation_score);
            ham   .push_back(stats[r].hamming_weight);
            v2_dummy.push_back(0.0); // avg_v2 dropped — replaced by avg_odd_mult
            trail1.push_back(stats[r].trailing_ones);
            tz3n1 .push_back(stats[r].trailing_zeros_3n1);
            m3    .push_back(stats[r].mod_3);
            m9    .push_back(stats[r].mod_9);
            m27   .push_back(stats[r].mod_27);
            ent   .push_back(stats[r].bit_entropy);
            aom   .push_back(avg_om);
        }

        static const std::vector<std::string> FEATURE_NAMES = {
            "max_run_1s", "alternation_score", "hamming_weight", "avg_v2_placeholder",
            "trailing_ones", "trailing_zeros_3n1", "mod_3", "mod_9", "mod_27",
            "bit_entropy", "avg_odd_mult"
        };

        std::vector<std::vector<double>> X_cols = {
            run1s, alts, ham, v2_dummy, trail1, tz3n1, m3, m9, m27, ent, aom
        };

        TrainingResult tr;
        tr.model           = fit_ols(X_cols, Y, FEATURE_NAMES);
        tr.class_avg_mult  = avg_mult_per_class;
        tr.modulo          = modulo;
        return tr;
    }

    // --------------------------------------------------------
    // predict_single: use trained model to predict one number
    // --------------------------------------------------------
    static double predict_single(const RegressionModel& model,
                                 unsigned long long n,
                                 int bit_width,
                                 double class_avg_mult)
    {
        NumberFeatures f = extract_features(n, bit_width);
        f.avg_odd_mult   = class_avg_mult;

        const auto& w = model.weights;
        double val = model.intercept;
        // match column order from compute_weights
        val += w[0]  * f.max_run_1s;
        val += w[1]  * f.alternation_score;
        val += w[2]  * f.hamming_weight;
        // w[3] is avg_v2_placeholder (0.0 column) — effectively zero weight
        val += w[4]  * f.trailing_ones;
        val += w[5]  * f.trailing_zeros_3n1;
        val += w[6]  * f.mod_3;
        val += w[7]  * f.mod_9;
        val += w[8]  * f.mod_27;
        val += w[9]  * f.bit_entropy;
        val += w[10] * f.avg_odd_mult;
        return val;
    }

    // Pearson correlation helper
    static double pearson(const std::vector<double>& X,
                          const std::vector<double>& Y)
    {
        int n = (int)X.size();
        if (n == 0 || n != (int)Y.size()) return 0.0;
        double sx = 0, sy = 0;
        for (int i = 0; i < n; ++i) { sx += X[i]; sy += Y[i]; }
        double mx = sx / n, my = sy / n;
        double num = 0, dx = 0, dy = 0;
        for (int i = 0; i < n; ++i) {
            num += (X[i] - mx) * (Y[i] - my);
            dx  += (X[i] - mx) * (X[i] - mx);
            dy  += (Y[i] - my) * (Y[i] - my);
        }
        return (dx < 1e-12 || dy < 1e-12) ? 0.0 : num / std::sqrt(dx * dy);
    }
};

} // namespace research
} // namespace collatz
