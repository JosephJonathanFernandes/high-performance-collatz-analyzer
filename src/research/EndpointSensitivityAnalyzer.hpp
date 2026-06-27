#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <algorithm>

namespace collatz {
namespace research {

class EndpointSensitivityAnalyzer {
private:
    struct DataPoint {
        double N;
        double mu;
    };

    struct ScenarioResult {
        std::string name;
        int points;
        double a;
        double b;
        double c;
        double delta_a;
        double pred_50B;
        double pred_100B;
        double delta_p50;
        double delta_p100;
    };

    static bool solve_3x3(const double M[3][3], const double V[3], double beta[3]) {
        double D = M[0][0] * (M[1][1] * M[2][2] - M[1][2] * M[2][1])
                 - M[0][1] * (M[1][0] * M[2][2] - M[1][2] * M[2][0])
                 + M[0][2] * (M[1][0] * M[2][1] - M[1][1] * M[2][0]);

        if (std::abs(D) < 1e-15) return false;

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

    static bool fit_model(const std::vector<DataPoint>& data, double& a, double& b, double& c) {
        double M[3][3] = {0};
        double V[3] = {0};
        for (const auto& pt : data) {
            double lN = std::log(pt.N);
            double x1 = 1.0 / lN;
            double x2 = 1.0 / (lN * lN);
            
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

public:
    static void analyze(const std::string& csv_file) {
        std::cout << "\n========================================================\n";
        std::cout << "Research Module 30: Endpoint Sensitivity Analyzer\n";
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
                    double N = std::stod(limit_str);
                    double mu_obs = std::stod(mu_str);
                    if (N > 1.0) {
                        data.push_back({N, mu_obs});
                    }
                } catch (...) {}
            }
        }

        int n = data.size();
        if (n < 4) {
            std::cerr << "[ERROR] Not enough data points.\n";
            return;
        }

        std::sort(data.begin(), data.end(), [](const DataPoint& pt_a, const DataPoint& pt_b) {
            return pt_a.N < pt_b.N;
        });

        auto eval_scenario = [](const std::string& name, const std::vector<DataPoint>& subset, double base_a, double base_p50, double base_p100) -> ScenarioResult {
            ScenarioResult res;
            res.name = name;
            res.points = subset.size();
            
            if (fit_model(subset, res.a, res.b, res.c)) {
                res.pred_50B = res.a + res.b / std::log(50000000000.0) + res.c / std::pow(std::log(50000000000.0), 2);
                res.pred_100B = res.a + res.b / std::log(100000000000.0) + res.c / std::pow(std::log(100000000000.0), 2);
                
                res.delta_a = res.a - base_a;
                res.delta_p50 = res.pred_50B - base_p50;
                res.delta_p100 = res.pred_100B - base_p100;
            } else {
                res.a = res.b = res.c = 0;
            }
            return res;
        };

        double base_a, base_b, base_c;
        if (!fit_model(data, base_a, base_b, base_c)) {
            std::cerr << "[ERROR] Baseline fit failed.\n";
            return;
        }
        double base_p50 = base_a + base_b / std::log(50000000000.0) + base_c / std::pow(std::log(50000000000.0), 2);
        double base_p100 = base_a + base_b / std::log(100000000000.0) + base_c / std::pow(std::log(100000000000.0), 2);

        std::vector<ScenarioResult> scenarios;
        
        scenarios.push_back(eval_scenario("Baseline (Full)", data, base_a, base_p50, base_p100));

        if (n > 2) {
            std::vector<DataPoint> sub(data.begin() + 2, data.end());
            scenarios.push_back(eval_scenario("Drop Early (2)", sub, base_a, base_p50, base_p100));
        }

        if (n > 2) {
            std::vector<DataPoint> sub(data.begin(), data.end() - 2);
            scenarios.push_back(eval_scenario("Drop Late (2)", sub, base_a, base_p50, base_p100));
        }

        if (n > 4) {
            std::vector<DataPoint> sub(data.begin() + 2, data.end() - 2);
            scenarios.push_back(eval_scenario("Drop Both Ends (4)", sub, base_a, base_p50, base_p100));
        }

        std::cout << "Endpoint Sensitivity Analysis\n";
        std::cout << "-------------------------------------------------------------------------------------------------\n";
        std::cout << std::left << std::setw(20) << "Scenario" 
                  << std::setw(15) << "New a" 
                  << std::setw(15) << "Delta a" 
                  << std::setw(15) << "Pred 50B" 
                  << std::setw(15) << "Delta 50B"
                  << std::setw(15) << "Pred 100B"
                  << "Delta 100B\n";
        std::cout << "-------------------------------------------------------------------------------------------------\n";

        for (const auto& res : scenarios) {
            std::cout << std::left << std::setw(20) << res.name
                      << std::setw(15) << std::fixed << std::setprecision(6) << res.a
                      << std::showpos << std::setw(15) << std::fixed << std::setprecision(6) << res.delta_a << std::noshowpos
                      << std::setw(15) << std::fixed << std::setprecision(6) << res.pred_50B
                      << std::showpos << std::setw(15) << std::fixed << std::setprecision(6) << res.delta_p50 << std::noshowpos
                      << std::setw(15) << std::fixed << std::setprecision(6) << res.pred_100B
                      << std::showpos << std::fixed << std::setprecision(6) << res.delta_p100 << std::noshowpos << "\n";
        }
        std::cout << "-------------------------------------------------------------------------------------------------\n\n";

        std::cout << "========================================================\n\n";
    }
};

} // namespace research
} // namespace collatz
