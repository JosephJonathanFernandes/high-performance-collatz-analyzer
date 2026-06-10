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

using namespace collatz;
using namespace collatz::research;
using namespace std;
using namespace std::chrono;

void print_help() {
    cout << "========================================================\n";
    cout << "   Collatz Computational Mathematics Research Platform  \n";
    cout << "========================================================\n";
    cout << "Usage: ./collatz.exe [module] [limit/depth] [modulo]\n\n";
    cout << "Available Modules:\n";
    cout << "  benchmark  <limit>          - Run extreme performance benchmarks\n";
    cout << "  tree       <depth>          - Generate the Reverse Collatz Tree\n";
    cout << "  growth     <start> <end>    - Fit logN(d) vs d using Least Squares\n";
    cout << "  oddtoodd   <limit>          - Analyze the Compressed Odd-to-Odd Map\n";
    cout << "  residue    <limit> <modulo> - Analyze residue classes (e.g. 16, 64)\n";
    cout << "  binary     <limit> <modulo> - Correlate Collatz difficulty with binary patterns\n";
    cout << "  near_cex   <limit>          - Detect near counterexamples (High R(n))\n";
    cout << "  graph      <limit>          - Build graph and run cycle detection\n";
    cout << "  stats      <limit>          - Verify v_2 probability distributions\n";
    cout << "  all        <limit>          - Run all modules sequentially\n";
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

    if (argc > 2) {
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
    else if (module == "all") {
        ReverseTreeExplorer::analyze(25); // Hardcoded depth for 'all' to prevent lockups
        OddToOddAnalyzer::analyze(limit);
        ResidueAnalyzer::analyze(limit, 64);
        NearCounterexampleDetector::analyze(limit, 10);
        GraphAnalyzer::analyze(limit);
        StatisticalAnalyzer::analyze(limit);
    } 
    else {
        print_help();
    }

    return 0;
}
