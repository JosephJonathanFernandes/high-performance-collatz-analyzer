#ifndef ODDTOODD_H
#define ODDTOODD_H

#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>

class OddToOddAnalyzer {
public:
    static void analyze(long long limit) {
        std::cout << "\n========================================================\n";
        std::cout << "Research Module 2: Compressed Odd-to-Odd Map Analyzer\n";
        std::cout << "Evaluating T(n) = (3n+1) / 2^(v_2(3n+1)) up to: " << limit << "\n";
        std::cout << "========================================================\n";

        auto start_time = std::chrono::high_resolution_clock::now();

        long long contractions = 0; // T(n) < n
        long long expansions = 0;   // T(n) > n
        long long equals = 0;       // T(n) == n (only 1)
        long long total_odds = 0;

        // Count how many times each v_2 happens. 
        // e.g. v_2_counts[x] = number of times we divide by 2^x
        std::vector<long long> v_2_counts(100, 0); 
        int max_v_2 = 0;
        double max_expansion_ratio = 0.0;
        long long max_expansion_node = 0;

        // We only care about odd numbers
        for (long long n = 3; n <= limit; n += 2) {
            total_odds++;

            long long next_n = 3 * n + 1;
            int v_2 = 0;
            
            // Extract powers of 2
            while ((next_n & 1) == 0) {
                next_n >>= 1;
                v_2++;
            }

            if (v_2 < 100) {
                v_2_counts[v_2]++;
            }
            if (v_2 > max_v_2) max_v_2 = v_2;

            if (next_n < n) {
                contractions++;
            } else if (next_n > n) {
                expansions++;
                double ratio = (double)next_n / (double)n;
                if (ratio > max_expansion_ratio) {
                    max_expansion_ratio = ratio;
                    max_expansion_node = n;
                }
            } else {
                equals++;
            }
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

        std::cout << "Total odd numbers analyzed : " << total_odds << "\n";
        std::cout << "Expansions [T(n) > n]      : " << expansions << " (" 
                  << std::fixed << std::setprecision(2) << ((double)expansions / total_odds) * 100.0 << "%)\n";
        std::cout << "Contractions [T(n) < n]    : " << contractions << " (" 
                  << std::fixed << std::setprecision(2) << ((double)contractions / total_odds) * 100.0 << "%)\n";
        
        std::cout << "\nMax Expansion Ratio observed : " << max_expansion_ratio 
                  << "x (at n = " << max_expansion_node << ")\n";

        std::cout << "\nDistribution of v_2 (Powers of 2 divided out):\n";
        for (int i = 1; i <= max_v_2 && i < 10; ++i) {
            std::cout << "  v_2 = " << i << " : " << v_2_counts[i] << " times ("
                      << std::fixed << std::setprecision(2) << ((double)v_2_counts[i] / total_odds) * 100.0 << "%)\n";
        }
        if (max_v_2 >= 10) {
            std::cout << "  v_2 >= 10: [long tail distribution omitted]\n";
        }

        std::cout << "\nExecution Time             : " << duration << " ms\n";
    }
};

#endif // ODDTOODD_H
