#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>
#include <stdexcept>
#include <windows.h>

// Core Engines
#include "collatz/CollatzMap.h"
#include "collatz/CollatzVector.h"
#include "collatz/CollatzMultiThread.h"

// Research Modules
#include "research/ReverseTreeExplorer.hpp"
#include "research/OddToOddAnalyzer.hpp"
#include "research/ResidueAnalyzer.hpp"
#include "research/NearCounterexampleDetector.hpp"
#include "research/GraphAnalyzer.hpp"
#include "research/StatisticalAnalyzer.hpp"
#include "research/DataExporter.hpp"
#include "research/GrowthEstimator.hpp"
#include "research/BinaryPatternAnalyzer.hpp"
// Phase 2 Modules
#include "research/CollatzFeatureEngine.hpp"
#include "research/DifficultyPredictor.hpp"
#include "research/OutlierEngine.hpp"
#include "research/ResidueEvolutionAnalyzer.hpp"
#include "research/FeatureImportanceAnalyzer.hpp"
#include "research/HeatmapGenerator.hpp"
#include "research/AdvancedBinaryAnalyzer.hpp"
#include "research/ReportGenerator.hpp"
#include "research/OddToOddDriftSpectrum.hpp"
#include "research/GrowthConvergenceAnalyzer.hpp"
#include "research/OutlierTrajectoryAnalyzer.hpp"
#include "research/TheoremCheckAnalyzer.hpp"
#include "research/V2MarkovAnalyzer.hpp"
#include "research/DriftLawAnalyzer.hpp"
#include "research/DriftConvergenceAnalyzer.hpp"

using namespace collatz;
using namespace collatz::research;
using namespace std;
using namespace std::chrono;

void print_help() {
    cout << "========================================================\n";
    cout << "   Collatz Computational Mathematics Research Platform  \n";
    cout << "========================================================\n";
    cout << "Usage: ./collatz.exe [module] [args...]\n\n";
    cout << "--- Core Modules ---\n";
    cout << "  benchmark          <limit>               Run extreme performance benchmarks\n";
    cout << "  tree               <depth>               Generate the Reverse Collatz Tree\n";
    cout << "  growth             <start> <end>         Fit logN(d) vs d using Least Squares\n";
    cout << "  oddtoodd           <limit>               Analyze the Compressed Odd-to-Odd Map\n";
    cout << "  residue            <limit> <modulo>      Analyze residue classes\n";
    cout << "  binary             <limit> <modulo>      Correlate difficulty with binary patterns\n";
    cout << "  near_cex           <limit>               Detect near counterexamples\n";
    cout << "  graph              <limit>               Build graph and run cycle detection\n";
    cout << "  stats              <limit>               Verify v_2 probability distributions\n";
    cout << "--- Predictive Modules ---\n";
    cout << "  predict            <limit>               Predict stopping times; compute MAE/MSE/R^2\n";
    cout << "  outliers           <limit>               Find numbers least explained by the model\n";
    cout << "  residue_evolution  <mod_start> <mod_end> Track hardest/easiest across moduli\n";
    cout << "  importance         <limit>               LOO + Permutation feature importance\n";
    cout << "  heatmap            <modulus>             Difficulty heatmap + Graphviz DOT\n";
    cout << "  advanced_binary    <limit> <modulo>      19-feature advanced binary analysis\n";
    cout << "  report                                   Auto-generate research_report.md\n";
    cout << "  drift_spectrum     <limit> <modulo>      E[log(T(n)/n)] drift spectrum analysis\n";
    cout << "  outlier_trajectory <number>              Analyze trajectory and v2 transitions\n";
    cout << "  theorem_check      <max_k>               Analytically test symbolic families\n";
    cout << "  v2_markov          <limit>               Global Markov transition matrix P(v2|v2)\n";
    cout << "  drift_law          <limit>               Test S(n) ≈ A + B*(log(n)/|mu_n|)\n";
    cout << "  drift_convergence  <limit>               Test if mu_k -> mu_infty as k increases\n";
    cout << "  k_convergence      <file> <start_depth>  Trace k(depth) stability\n";
    cout << "  all                <limit>               Run all core modules sequentially\n";
    cout << "========================================================\n";
}

int main(int argc, char* argv[]) {
    ios::sync_with_stdio(false);
    cin.tie(NULL);

    DataExporter::init_directories();

    if (argc < 2) {
        print_help();
        return 0;
    }

    string module = argv[1];
    unsigned long long limit = 1000000;

    if (argc > 2 && module != "report" && module != "k_convergence") {
        try {
            limit = stoull(argv[2]);
        } catch (...) {
            cerr << "[ERROR] Invalid numeric limit provided.\n";
            return 1;
        }
    }

    if (module == "benchmark") {
        CollatzMultiThread mt(12);
        auto start = high_resolution_clock::now();
        auto res = mt.find_longest_chain(limit);
        auto duration = duration_cast<milliseconds>(high_resolution_clock::now() - start).count();
        cout << "Longest Start: " << res.starting_number << " | Steps: " << res.steps << " | Time: " << duration << "ms\n";
    } 
    else if (module == "tree") {
        ReverseTreeExplorer::analyze(static_cast<int>(limit));
    } 
    else if (module == "oddtoodd") {
        OddToOddAnalyzer::analyze(limit);
    } 
    else if (module == "residue") {
        unsigned long long modulo = (argc > 3) ? stoull(argv[3]) : 64;
        ResidueAnalyzer::analyze(limit, modulo);
    } 
    else if (module == "binary") {
        unsigned long long modulo = (argc > 3) ? stoull(argv[3]) : 256;
        BinaryPatternAnalyzer::analyze(limit, modulo);
    } 
    else if (module == "near_cex") {
        NearCounterexampleDetector::analyze(limit, 100);
    } 
    else if (module == "graph") {
        GraphAnalyzer::analyze(limit);
    } 
    else if (module == "stats") {
        StatisticalAnalyzer::analyze(limit);
    } 
    else if (module == "growth") {
        int start_depth = (argc > 2) ? stoi(argv[2]) : 30;
        int end_depth = (argc > 3) ? stoi(argv[3]) : 70;
        string file = (argc > 4) ? argv[4] : "data/csv/reverse_tree_70.csv";
        GrowthEstimator::analyze(file, start_depth, end_depth);
    } 
    else if (module == "predict") {
        DifficultyPredictor::analyze(limit);
    }
    else if (module == "outliers") {
        int top_k = (argc > 3) ? stoi(argv[3]) : 100;
        OutlierEngine::analyze(limit, top_k);
    }
    else if (module == "residue_evolution") {
        unsigned long long mod_end  = (argc > 3) ? stoull(argv[3]) : 1024;
        unsigned long long ev_limit = (argc > 4) ? stoull(argv[4]) : 10000000;
        ResidueEvolutionAnalyzer::analyze(limit, mod_end, ev_limit);
    }
    else if (module == "importance") {
        FeatureImportanceAnalyzer::analyze(limit);
    }
    else if (module == "heatmap") {
        unsigned long long hm_limit = (argc > 3) ? stoull(argv[3]) : 10000000;
        HeatmapGenerator::analyze(limit, hm_limit);
    }
    else if (module == "advanced_binary") {
        unsigned long long modulo = (argc > 3) ? stoull(argv[3]) : 1024;
        AdvancedBinaryAnalyzer::analyze(limit, modulo);
    }
    else if (module == "report") {
        ReportGenerator::generate();
    }
    else if (module == "drift_spectrum") {
        unsigned long long modulo = (argc > 3) ? stoull(argv[3]) : 1024;
        OddToOddDriftSpectrum::analyze(limit, modulo);
    }
    else if (module == "k_convergence") {
        string file = (argc > 2) ? argv[2] : "data/csv/reverse_tree_100.csv";
        int start_depth = (argc > 3) ? stoi(argv[3]) : 30;
        GrowthConvergenceAnalyzer::analyze(file, start_depth);
    }
    else if (module == "outlier_trajectory") {
        OutlierTrajectoryAnalyzer::analyze(limit);
    }
    else if (module == "theorem_check") {
        int max_k = (argc > 2) ? stoi(argv[2]) : 60;
        TheoremCheckAnalyzer::analyze(max_k);
    }
    else if (module == "v2_markov") {
        V2MarkovAnalyzer::analyze(limit);
    }
    else if (module == "drift_law") {
        DriftLawAnalyzer::analyze(limit);
    }
    else if (module == "drift_convergence") {
        DriftConvergenceAnalyzer::analyze(limit);
    }
    else if (module == "all") {
        std::cout << "Running the 7 Core Research Paper Modules at limit: " << limit << "\n\n";
        DriftLawAnalyzer::analyze(limit);
        OddToOddDriftSpectrum::analyze(limit, 1024);
        DriftConvergenceAnalyzer::analyze(limit);
        V2MarkovAnalyzer::analyze(limit);
        ResidueAnalyzer::analyze(limit, 64);
        ResidueEvolutionAnalyzer::analyze(limit, 64, 4096);
        ReverseTreeExplorer::analyze(30);
    } 
    else {
        print_help();
    }

    return 0;
}
