#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <ctime>

namespace collatz {
namespace research {

class PredictionAuditor {
private:
    struct Prediction {
        unsigned long long target_N;
        double predicted_mu;
        bool verified;
        double actual_mu;
        int prediction_number;
        double loocv_mae;
    };

    static std::string format_N(unsigned long long N) {
        if (N >= 1000000000) return std::to_string(N / 1000000000) + "B";
        if (N >= 1000000) return std::to_string(N / 1000000) + "M";
        return std::to_string(N);
    }

    static std::string remove_commas(const std::string& str) {
        std::string res;
        for (char c : str) {
            if (c != ',') res += c;
        }
        return res;
    }

    static std::map<unsigned long long, double> load_actuals(const std::string& csv_file) {
        std::map<unsigned long long, double> actuals;
        std::ifstream file(csv_file);
        if (!file.is_open()) return actuals;

        std::string line;
        std::getline(file, line); // header
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            std::stringstream ss(line);
            std::string limit_str, mu_str;
            if (std::getline(ss, limit_str, ',') && std::getline(ss, mu_str, ',')) {
                try {
                    actuals[std::stoull(limit_str)] = std::stod(mu_str);
                } catch (...) {}
            }
        }
        return actuals;
    }

public:
    static void analyze(const std::string& predictions_file, const std::string& actuals_file) {
        std::cout << "\n========================================================\n";
        std::cout << "Research Module 23: Prediction Auditor\n";
        std::cout << "========================================================\n\n";

        auto actuals = load_actuals(actuals_file);

        std::ifstream file(predictions_file);
        if (!file.is_open()) {
            std::cerr << "[ERROR] Could not open " << predictions_file << "\n";
            return;
        }

        std::vector<Prediction> predictions;
        std::string line;
        int current_pred_num = 0;
        unsigned long long current_target = 0;
        double current_pred_mu = 0.0;
        bool in_record = false;

        while (std::getline(file, line)) {
            if (line.find("**Blind Prediction Record #") != std::string::npos) {
                size_t pos = line.find("#");
                if (pos != std::string::npos) {
                    try {
                        current_pred_num = std::stoi(line.substr(pos + 1));
                        in_record = true;
                    } catch(...) {}
                }
            } else if (in_record && line.find("Target limit") != std::string::npos) {
                size_t pos = line.find(":");
                if (pos != std::string::npos) {
                    std::string val = line.substr(pos + 1);
                    val.erase(std::remove_if(val.begin(), val.end(), ::isspace), val.end());
                    try {
                        current_target = std::stoull(remove_commas(val));
                    } catch(...) {}
                }
            } else if (in_record && (line.find("Predicted μ(") != std::string::npos || line.find("Predicted mu(") != std::string::npos)) {
                size_t pos = line.find(":");
                if (pos != std::string::npos) {
                    std::string val = line.substr(pos + 1);
                    val.erase(std::remove_if(val.begin(), val.end(), ::isspace), val.end());
                    try {
                        current_pred_mu = std::stod(val);
                        
                        Prediction p;
                        p.prediction_number = current_pred_num;
                        p.target_N = current_target;
                        p.predicted_mu = current_pred_mu;
                        
                        if (actuals.find(current_target) != actuals.end()) {
                            p.verified = true;
                            p.actual_mu = actuals[current_target];
                        } else {
                            p.verified = false;
                            p.actual_mu = 0.0;
                        }
                        
                        predictions.push_back(p);
                        in_record = false;
                    } catch(...) {}
                }
            } else if (line.find("LOOCV MAE") != std::string::npos) {
                if (!predictions.empty()) {
                    size_t pos = line.find(":");
                    if (pos != std::string::npos) {
                        std::string val = line.substr(pos + 1);
                        val.erase(std::remove_if(val.begin(), val.end(), ::isspace), val.end());
                        try {
                            predictions.back().loocv_mae = std::stod(val);
                        } catch(...) {}
                    }
                }
            }
        }

        if (predictions.empty()) {
            std::cout << "No predictions found in ledger.\n";
            return;
        }

        int verified_count = 0;
        double sum_abs_error = 0.0;
        double max_error = 0.0;

        for (const auto& p : predictions) {
            std::cout << "Prediction #" << p.prediction_number << "\n";
            std::cout << std::left << std::setw(16) << "Target N" << ": " << p.target_N << "\n";
            std::cout << std::setw(16) << "Predicted mu" << ": " << std::fixed << std::setprecision(8) << p.predicted_mu << "\n";
            
            if (p.verified) {
                double residual = p.actual_mu - p.predicted_mu;
                double abs_err = std::abs(residual);
                verified_count++;
                sum_abs_error += abs_err;
                if (abs_err > max_error) max_error = abs_err;

                std::cout << std::setw(16) << "Actual mu" << ": " << std::fixed << std::setprecision(8) << p.actual_mu << "\n";
                std::cout << std::setw(16) << "Residual" << ": " << std::scientific << std::setprecision(8) << residual << "\n";
                std::cout << std::setw(16) << "Absolute Error" << ": " << std::scientific << std::setprecision(8) << abs_err << "\n";
                std::cout << std::setw(16) << "Status" << ": Verified\n\n";
            } else {
                std::cout << std::setw(16) << "Actual mu" << ": ---\n";
                std::cout << std::setw(16) << "Residual" << ": ---\n";
                std::cout << std::setw(16) << "Absolute Error" << ": ---\n";
                std::cout << std::setw(16) << "Status" << ": Pending\n\n";
            }
        }

        std::cout << "Prediction Confidence\n\n";
        
        double historical_mae = 0.0;
        double historical_worst = 0.0;
        double empirical_bound = 0.0;
        
        if (verified_count > 0) {
            historical_mae = sum_abs_error / verified_count;
            historical_worst = max_error;
            
            double sum_res = 0.0;
            for (const auto& p : predictions) {
                if (p.verified) sum_res += (p.actual_mu - p.predicted_mu);
            }
            double mean_res = sum_res / verified_count;
            
            double sum_sq_diff = 0.0;
            double fallback_loocv = 0.0;
            for (const auto& p : predictions) {
                if (p.verified) {
                    double res = (p.actual_mu - p.predicted_mu);
                    sum_sq_diff += (res - mean_res) * (res - mean_res);
                    if (p.loocv_mae > fallback_loocv) fallback_loocv = p.loocv_mae;
                }
            }
            
            if (verified_count > 1) {
                double std_residuals = std::sqrt(sum_sq_diff / (verified_count - 1));
                empirical_bound = 1.96 * std_residuals;
            } else {
                empirical_bound = 2.0 * std::max(historical_mae, fallback_loocv);
            }
            
            std::cout << "Based on historical prediction performance:\n\n";
            std::cout << "Mean abs error      : " << std::scientific << std::setprecision(8) << historical_mae << "\n";
            std::cout << "Worst error         : " << std::scientific << std::setprecision(8) << historical_worst << "\n";
            std::cout << "95% empirical bound : " << std::scientific << std::setprecision(8) << empirical_bound << "\n\n";
        } else {
            std::cout << "No verified prediction history available.\n";
            std::cout << "Current uncertainty uses LOOCV proxy only.\n\n";
            std::cout << "Estimated range (proxy): mu +/- 2 * LOOCV_MAE\n\n";
        }
        
        for (const auto& p : predictions) {
            if (!p.verified) {
                double p_bound = (verified_count > 0) ? empirical_bound : (p.loocv_mae * 2.0);
                std::cout << format_N(p.target_N) << " prediction:\n";
                std::cout << "mu = " << std::fixed << std::setprecision(8) << p.predicted_mu 
                          << " +/- " << std::scientific << std::setprecision(8) << p_bound << "\n\n";
            }
        }
        
        std::cout << "Overall:\n";
        std::cout << "Predictions tested : " << verified_count << "\n";
        std::cout << "========================================================\n\n";
        
        // Export to CSV
        std::string out_path = "data/csv/prediction_audit.csv";
        std::ofstream out_csv(out_path);
        if (out_csv.is_open()) {
            out_csv << "prediction_id,target_N,predicted_mu,actual_mu,residual,absolute_error,status\n";
            for (const auto& p : predictions) {
                out_csv << p.prediction_number << ","
                        << p.target_N << ","
                        << std::fixed << std::setprecision(8) << p.predicted_mu << ",";
                
                if (p.verified) {
                    double residual = p.actual_mu - p.predicted_mu;
                    double abs_err = std::abs(residual);
                    out_csv << std::fixed << std::setprecision(8) << p.actual_mu << ","
                            << std::scientific << std::setprecision(8) << residual << ","
                            << std::scientific << std::setprecision(8) << abs_err << ","
                            << "Verified\n";
                } else {
                    out_csv << ",,," << "Pending\n";
                }
            }
            out_csv.close();
            std::cout << "Audit data exported to : " << out_path << "\n";
        } else {
            std::cerr << "[ERROR] Could not write " << out_path << "\n";
        }
    }

    static void add_prediction(const std::string& predictions_file, unsigned long long N, double predicted_mu, double loocv_mae) {
        // Read existing file to find the next Prediction Record #
        std::ifstream in(predictions_file);
        int next_id = 1;
        std::string line;
        while (std::getline(in, line)) {
            if (line.find("**Blind Prediction Record #") != std::string::npos) {
                size_t pos = line.find("#");
                if (pos != std::string::npos) {
                    try {
                        int id = std::stoi(line.substr(pos + 1));
                        if (id >= next_id) next_id = id + 1;
                    } catch(...) {}
                }
            }
        }
        in.close();

        // Get current date
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::ostringstream date_oss;
        date_oss << std::put_time(&tm, "%Y-%m-%d");

        std::ofstream out(predictions_file, std::ios_base::app);
        if (!out.is_open()) {
            std::cerr << "[ERROR] Could not open " << predictions_file << " for appending.\n";
            return;
        }

        out << "\n**Blind Prediction Record #" << next_id << "**\n\n";
        out << "```text\n";
        out << "Target limit      : " << N << "\n";
        out << "Prediction date   : " << date_oss.str() << "\n";
        out << "Model             : CLI Added\n\n";
        out << "Predicted μ       : " << std::fixed << std::setprecision(8) << predicted_mu << "\n";
        if (loocv_mae > 0.0) {
            out << "LOOCV MAE         : " << std::scientific << std::setprecision(8) << loocv_mae << "\n";
        }
        out << "```\n";
        out.close();

        std::cout << "Added Blind Prediction Record #" << next_id << " for N=" << N << " to ledger.\n";
    }

    static void verify_prediction(const std::string& predictions_file, const std::string& actuals_file, unsigned long long N, double actual_mu) {
        std::cout << "\n========================================================\n";
        std::cout << "Module 24: Prediction Verification Engine\n";
        std::cout << "========================================================\n\n";

        // Check if N already exists in actuals
        auto actuals = load_actuals(actuals_file);
        if (actuals.find(N) == actuals.end()) {
            std::ofstream out(actuals_file, std::ios_base::app);
            if (out.is_open()) {
                out << N << "," << std::fixed << std::setprecision(8) << actual_mu << "\n";
                out.close();
                std::cout << "Appended empirical result N=" << N << " to " << actuals_file << ".\n";
            }
        } else {
            std::cout << "Result for N=" << N << " already exists in CSV.\n";
        }
        
        std::cout << "\nTriggering Verification Audit...\n";
        analyze(predictions_file, actuals_file);
    }
};

} // namespace research
} // namespace collatz
