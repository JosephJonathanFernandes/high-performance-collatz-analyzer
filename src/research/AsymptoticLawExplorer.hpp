#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <functional>

namespace collatz {
namespace research {

class AsymptoticLawExplorer {
private:
    struct DataPoint {
        double N;
        double mu;
    };

    struct ModelResult {
        std::string name;
        int k;
        double r_squared;
        double aic;
        double bic;
        double loocv_mae;
        double best_a;
        double best_b;
        double best_c;
    };

    static bool solve_2x2(const double M[2][2], const double V[2], double beta[2]) {
        double D = M[0][0] * M[1][1] - M[0][1] * M[1][0];
        if (std::abs(D) < 1e-12) return false;
        beta[0] = (V[0] * M[1][1] - M[0][1] * V[1]) / D;
        beta[1] = (M[0][0] * V[1] - V[0] * M[1][0]) / D;
        return true;
    }

    static bool solve_3x3(const double M[3][3], const double V[3], double beta[3]) {
        double D = M[0][0] * (M[1][1] * M[2][2] - M[1][2] * M[2][1])
                 - M[0][1] * (M[1][0] * M[2][2] - M[1][2] * M[2][0])
                 + M[0][2] * (M[1][0] * M[2][1] - M[1][1] * M[2][0]);

        if (std::abs(D) < 1e-12) return false;

        double Da = V[0] * (M[1][1] * M[2][2] - M[1][2] * M[2][1])
                  - M[0][1] * (V[1] * M[2][2] - M[1][2] * V[2])
                  + M[0][2] * (V[1] * M[2][1] - M[1][1] * V[2]);

        double Db = M[0][0] * (V[1] * M[2][2] - M[1][2] * V[2])
                  - V[0] * (M[1][0] * M[2][2] - M[1][2] * M[2][0])
                  + M[0][2] * (M[1][0] * V[2] - V[1] * M[2][0]);

        double Dc = M[0][0] * (M[1][1] * V[2] - V[1] * M[2][1])
                  - M[0][1] * (M[1][0] * V[2] - V[1] * M[2][0])
                  + V[0] * (M[1][0] * M[2][1] - M[1][1] * M[2][0]);

        beta[0] = Da / D;
        beta[1] = Db / D;
        beta[2] = Dc / D;
        return true;
    }

    static bool fit_2param(const std::vector<DataPoint>& data, std::function<double(double)> tx, double& a, double& b) {
        double M[2][2] = {0};
        double V[2] = {0};
        for (const auto& pt : data) {
            double x = tx(pt.N);
            M[0][0] += 1;
            M[0][1] += x;
            M[1][1] += x * x;
            V[0] += pt.mu;
            V[1] += x * pt.mu;
        }
        M[1][0] = M[0][1];
        double beta[2];
        if (!solve_2x2(M, V, beta)) return false;
        a = beta[0];
        b = beta[1];
        return true;
    }

    static bool fit_3param(const std::vector<DataPoint>& data, std::function<double(double)> tx1, std::function<double(double)> tx2, double& a, double& b, double& c) {
        double M[3][3] = {0};
        double V[3] = {0};
        for (const auto& pt : data) {
            double x1 = tx1(pt.N);
            double x2 = tx2(pt.N);
            M[0][0] += 1;
            M[0][1] += x1;
            M[0][2] += x2;
            M[1][1] += x1 * x1;
            M[1][2] += x1 * x2;
            M[2][2] += x2 * x2;
            V[0] += pt.mu;
            V[1] += x1 * pt.mu;
            V[2] += x2 * pt.mu;
        }
        M[1][0] = M[0][1];
        M[2][0] = M[0][2];
        M[2][1] = M[1][2];
        double beta[3];
        if (!solve_3x3(M, V, beta)) return false;
        a = beta[0];
        b = beta[1];
        c = beta[2];
        return true;
    }

    static ModelResult evaluate_2param(const std::string& name, const std::vector<DataPoint>& data, std::function<double(double)> tx) {
        ModelResult res;
        res.name = name;
        res.k = 2;
        int n = data.size();
        
        double a = 0, b = 0;
        if (!fit_2param(data, tx, a, b)) return res;
        res.best_a = a;
        res.best_b = b;
        res.best_c = 0;

        double mean_mu = 0;
        for (const auto& pt : data) mean_mu += pt.mu;
        mean_mu /= n;

        double ss_tot = 0, ss_res = 0;
        for (const auto& pt : data) {
            double pred = a + b * tx(pt.N);
            ss_res += (pt.mu - pred) * (pt.mu - pred);
            ss_tot += (pt.mu - mean_mu) * (pt.mu - mean_mu);
        }
        res.r_squared = 1.0 - (ss_res / ss_tot);
        res.aic = n * std::log(ss_res / n) + 2.0 * res.k;
        res.bic = n * std::log(ss_res / n) + res.k * std::log(n);

        double mae = 0;
        for (int i = 0; i < n; ++i) {
            std::vector<DataPoint> subset;
            for (int j = 0; j < n; ++j) {
                if (i != j) subset.push_back(data[j]);
            }
            double a_loo, b_loo;
            if (fit_2param(subset, tx, a_loo, b_loo)) {
                double pred = a_loo + b_loo * tx(data[i].N);
                mae += std::abs(data[i].mu - pred);
            }
        }
        res.loocv_mae = mae / n;
        return res;
    }

    static ModelResult evaluate_3param(const std::string& name, const std::vector<DataPoint>& data, std::function<double(double)> tx1, std::function<double(double)> tx2) {
        ModelResult res;
        res.name = name;
        res.k = 3;
        int n = data.size();
        
        double a = 0, b = 0, c = 0;
        if (!fit_3param(data, tx1, tx2, a, b, c)) return res;
        res.best_a = a;
        res.best_b = b;
        res.best_c = c;

        double mean_mu = 0;
        for (const auto& pt : data) mean_mu += pt.mu;
        mean_mu /= n;

        double ss_tot = 0, ss_res = 0;
        for (const auto& pt : data) {
            double pred = a + b * tx1(pt.N) + c * tx2(pt.N);
            ss_res += (pt.mu - pred) * (pt.mu - pred);
            ss_tot += (pt.mu - mean_mu) * (pt.mu - mean_mu);
        }
        res.r_squared = 1.0 - (ss_res / ss_tot);
        res.aic = n * std::log(ss_res / n) + 2.0 * res.k;
        res.bic = n * std::log(ss_res / n) + res.k * std::log(n);

        double mae = 0;
        for (int i = 0; i < n; ++i) {
            std::vector<DataPoint> subset;
            for (int j = 0; j < n; ++j) {
                if (i != j) subset.push_back(data[j]);
            }
            double a_loo, b_loo, c_loo;
            if (fit_3param(subset, tx1, tx2, a_loo, b_loo, c_loo)) {
                double pred = a_loo + b_loo * tx1(data[i].N) + c_loo * tx2(data[i].N);
                mae += std::abs(data[i].mu - pred);
            }
        }
        res.loocv_mae = mae / n;
        return res;
    }

public:
    static void analyze(const std::string& csv_file) {
        std::cout << "\n========================================================\n";
        std::cout << "Research Module 25: Asymptotic Law Explorer\n";
        std::cout << "========================================================\n\n";

        std::ifstream file(csv_file);
        if (!file.is_open()) {
            std::cerr << "[ERROR] Could not open file: " << csv_file << "\n";
            return;
        }

        std::vector<DataPoint> data;
        std::string line;
        
        std::getline(file, line);

        while (std::getline(file, line)) {
            if (line.empty()) continue;
            std::stringstream ss(line);
            std::string limit_str, mu_str;
            if (std::getline(ss, limit_str, ',') && std::getline(ss, mu_str, ',')) {
                try {
                    double n_val = std::stod(limit_str);
                    double mu_val = std::stod(mu_str);
                    if (n_val > 1.0) {
                        data.push_back({n_val, mu_val});
                    }
                } catch (...) {}
            }
        }

        int n_pts = data.size();
        if (n_pts < 4) {
            std::cerr << "[ERROR] Not enough valid data points found in CSV.\n";
            return;
        }

        std::vector<ModelResult> results;

        results.push_back(evaluate_2param("1/log(N)", data, [](double N) { return 1.0 / std::log(N); }));
        results.push_back(evaluate_2param("1/log^2(N)", data, [](double N) { return 1.0 / (std::log(N) * std::log(N)); }));
        results.push_back(evaluate_2param("1/sqrt(log(N))", data, [](double N) { return 1.0 / std::sqrt(std::log(N)); }));
        results.push_back(evaluate_2param("1/log(N*log(N))", data, [](double N) { return 1.0 / std::log(N * std::log(N)); }));
        results.push_back(evaluate_2param("1/N^0.5", data, [](double N) { return 1.0 / std::sqrt(N); }));
        results.push_back(evaluate_2param("1/N", data, [](double N) { return 1.0 / N; }));
        
        results.push_back(evaluate_3param("1/log(N) + 1/log^2(N)", data, 
            [](double N) { return 1.0 / std::log(N); },
            [](double N) { return 1.0 / (std::log(N) * std::log(N)); }
        ));

        std::sort(results.begin(), results.end(), [](const ModelResult& a, const ModelResult& b) {
            return a.loocv_mae < b.loocv_mae;
        });

        std::cout << "Candidate Model Ranking\n";
        std::cout << "--------------------------------------------------------------------------------\n";
        std::cout << std::left << std::setw(25) << "Model" 
                  << std::setw(12) << "R^2" 
                  << std::setw(15) << "LOOCV MAE" 
                  << std::setw(15) << "AIC" 
                  << "BIC\n";
        std::cout << "--------------------------------------------------------------------------------\n";
        
        for (const auto& res : results) {
            std::cout << std::left << std::setw(25) << res.name
                      << std::setw(12) << std::fixed << std::setprecision(5) << res.r_squared
                      << std::setw(15) << std::scientific << std::setprecision(4) << res.loocv_mae
                      << std::setw(15) << std::fixed << std::setprecision(2) << res.aic
                      << std::setw(15) << std::fixed << std::setprecision(2) << res.bic << "\n";
        }
        std::cout << "--------------------------------------------------------------------------------\n\n";

        std::cout << "Winner (Based on LOOCV MAE):\n";
        if (results[0].k == 2) {
            std::cout << "mu(N) = a + b*(" << results[0].name << ")\n";
            std::cout << "a = " << std::fixed << std::setprecision(6) << results[0].best_a << "\n";
            std::cout << "b = " << std::fixed << std::setprecision(6) << results[0].best_b << "\n";
        } else {
            std::cout << "mu(N) = a + b/log(N) + c/log^2(N)\n";
            std::cout << "a = " << std::fixed << std::setprecision(6) << results[0].best_a << "\n";
            std::cout << "b = " << std::fixed << std::setprecision(6) << results[0].best_b << "\n";
            std::cout << "c = " << std::fixed << std::setprecision(6) << results[0].best_c << "\n";
        }
        std::cout << "========================================================\n\n";
    }
};

} // namespace research
} // namespace collatz
