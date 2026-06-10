/**
 * Collatz Sequence Optimizer
 * 
 * Computes the number under a given limit that produces the longest Collatz sequence.
 * 
 * Features:
 * - Iterative generation (no recursion to prevent stack overflow)
 * - Dynamic Programming (memoization) via unordered_map and vector
 * - Path compression to cache intermediate values efficiently
 * - Bitwise operations for fast math operations
 * - Fast I/O
 * - Performance benchmarking using std::chrono
 */

#include <iostream>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <algorithm>
#include <iomanip>
#include <string>
#include <windows.h> // Using Win32 threading to support older MinGW builds without std::thread
#include "ReverseTree.h"
#include "OddToOdd.h"

using namespace std;
using namespace std::chrono;

// Struct to hold the result of the Collatz calculation
struct CollatzResult {
    long long starting_number;
    int steps;
    long long peak_value;
};

// ============================================================================
// Implementation 1: Using unordered_map for memoization
// ============================================================================
class CollatzMap {
private:
    unordered_map<long long, int> cache;

public:
    CollatzMap() {
        cache[1] = 0; // Base case
    }

    CollatzResult find_longest_chain(long long limit) {
        long long longest_start = 1;
        int max_steps = 0;
        long long global_peak = 1;

        for (long long i = 1; i <= limit; ++i) {
            long long current = i;
            int steps = 0;
            long long peak = current;
            
            // Vector for path compression
            // Pre-allocate a reasonable size to avoid reallocations
            vector<long long> path;
            path.reserve(1000); 

            while (current != 1) {
                auto it = cache.find(current);
                if (it != cache.end()) {
                    steps += it->second;
                    break;
                }
                
                path.push_back(current);
                
                // Bitwise optimization: 
                // (current & 1) == 0 is equivalent to current % 2 == 0
                // current >>= 1 is equivalent to current /= 2
                if ((current & 1) == 0) {
                    current >>= 1; 
                } else {
                    current = current * 3 + 1;
                }
                
                if (current > peak) {
                    peak = current;
                }
                steps++;
            }

            // Path compression: back-propagate results to all numbers in the path
            int remaining_steps = steps;
            for (long long node : path) {
                cache[node] = remaining_steps;
                remaining_steps--;
            }

            if (steps > max_steps) {
                max_steps = steps;
                longest_start = i;
            }
            if (peak > global_peak) {
                global_peak = peak;
            }
        }

        return {longest_start, max_steps, global_peak};
    }
};

// ============================================================================
// Implementation 2: Using vector for bounded limits
// ============================================================================
// This approach is generally significantly faster due to better cache locality
// and avoiding hash function overhead, though it uses contiguous memory.
class CollatzVector {
private:
    vector<int> cache;
    long long cache_limit;

public:
    // Initialize cache up to max_limit.
    // Note: Numbers can exceed max_limit during the sequence, so we only cache
    // values that fall within our predefined cache bounds.
    CollatzVector(long long max_limit) : cache_limit(max_limit) {
        cache.assign(cache_limit + 1, -1);
        cache[1] = 0; // Base case
    }

    CollatzResult find_longest_chain(long long limit) {
        long long longest_start = 1;
        int max_steps = 0;
        long long global_peak = 1;

        for (long long i = 1; i <= limit; ++i) {
            long long current = i;
            int steps = 0;
            long long peak = current;
            
            vector<long long> path;
            path.reserve(1000);

            while (current != 1) {
                // Read from cache if within bounds
                if (current <= cache_limit && cache[current] != -1) {
                    steps += cache[current];
                    break;
                }

                path.push_back(current);

                if ((current & 1) == 0) {
                    current >>= 1;
                } else {
                    current = current * 3 + 1;
                }

                if (current > peak) {
                    peak = current;
                }
                steps++;
            }

            // Path compression
            int remaining_steps = steps;
            for (long long node : path) {
                // Write to cache only if within bounds
                if (node <= cache_limit) {
                    cache[node] = remaining_steps;
                }
                remaining_steps--;
            }

            if (steps > max_steps) {
                max_steps = steps;
                longest_start = i;
            }
            if (peak > global_peak) {
                global_peak = peak;
            }
        }

        return {longest_start, max_steps, global_peak};
    }
};

// ============================================================================
// Implementation 3: Multithreaded using Win32 API and chunking
// ============================================================================
// We use Windows native threading (CreateThread) to avoid compilation issues 
// on MinGW distributions that lack POSIX thread (std::thread) support.

struct ThreadData {
    long long start_val;
    long long end_val;
    long long limit;
    CollatzResult result;
};

DWORD WINAPI CollatzThreadWorker(LPVOID lpParam) {
    ThreadData* data = (ThreadData*)lpParam;
    
    // Thread-local cache: cap size to prevent massive memory usage per thread
    long long local_cache_limit = min(data->limit, 5000000LL); 
    vector<int> local_cache(local_cache_limit + 1, -1);
    local_cache[1] = 0;

    long long local_longest_start = data->start_val;
    int local_max_steps = 0;
    long long local_global_peak = 1;

    for (long long i = data->start_val; i <= data->end_val; ++i) {
        long long current = i;
        int steps = 0;
        long long peak = current;
        
        vector<long long> path;
        path.reserve(1000);

        while (current != 1) {
            if (current <= local_cache_limit && local_cache[current] != -1) {
                steps += local_cache[current];
                break;
            }

            path.push_back(current);

            if ((current & 1) == 0) {
                current >>= 1;
            } else {
                current = current * 3 + 1;
            }

            if (current > peak) {
                peak = current;
            }
            steps++;
        }

        int remaining_steps = steps;
        for (long long node : path) {
            if (node <= local_cache_limit) {
                local_cache[node] = remaining_steps;
            }
            remaining_steps--;
        }

        if (steps > local_max_steps) {
            local_max_steps = steps;
            local_longest_start = i;
        }
        if (peak > local_global_peak) {
            local_global_peak = peak;
        }
    }

    data->result = {local_longest_start, local_max_steps, local_global_peak};
    return 0;
}

class CollatzMultiThread {
private:
    int num_threads;

public:
    CollatzMultiThread(int threads) : num_threads(threads) {}

    CollatzResult find_longest_chain(long long limit) {
        if (limit < num_threads) num_threads = limit;
        
        long long chunk_size = limit / num_threads;
        
        vector<HANDLE> threads(num_threads);
        vector<ThreadData> thread_data(num_threads);

        for (int t = 0; t < num_threads; ++t) {
            thread_data[t].start_val = t * chunk_size + 1;
            thread_data[t].end_val = (t == num_threads - 1) ? limit : (thread_data[t].start_val + chunk_size - 1);
            thread_data[t].limit = limit;

            threads[t] = CreateThread(
                NULL,                   // default security attributes
                0,                      // use default stack size  
                CollatzThreadWorker,    // thread function name
                &thread_data[t],        // argument to thread function 
                0,                      // use default creation flags 
                NULL);                  // returns the thread identifier 
        }

        // Wait until all threads have terminated.
        WaitForMultipleObjects(num_threads, threads.data(), TRUE, INFINITE);

        long long best_start = 1;
        int best_steps = 0;
        long long best_peak = 1;

        for (int t = 0; t < num_threads; ++t) {
            CollatzResult res = thread_data[t].result;
            if (res.steps > best_steps) {
                best_steps = res.steps;
                best_start = res.starting_number;
            }
            if (res.peak_value > best_peak) {
                best_peak = res.peak_value;
            }
            CloseHandle(threads[t]);
        }

        return {best_start, best_steps, best_peak};
    }
};

// ============================================================================
// Main Execution & Benchmarking
// ============================================================================
void run_benchmark(long long limit) {
    cout << "========================================================\n";
    cout << "Collatz Sequence Optimization Benchmark\n";
    cout << "Search Limit: " << limit << "\n";
    cout << "========================================================\n";

    long long duration_map = -1, duration_vec = -1, duration_mt = -1;

    // --- Benchmark 1: unordered_map ---
    cout << "\n[1] Running unordered_map implementation...\n";
    try {
        CollatzMap map_solver;
        auto start_map = high_resolution_clock::now();
        CollatzResult result_map = map_solver.find_longest_chain(limit);
        auto end_map = high_resolution_clock::now();
        duration_map = duration_cast<milliseconds>(end_map - start_map).count();

        cout << "    Longest sequence generated by : " << result_map.starting_number << "\n";
        cout << "    Total steps                   : " << result_map.steps << "\n";
        cout << "    Global peak value reached     : " << result_map.peak_value << "\n";
        cout << "    Execution time                : " << duration_map << " ms\n";
    } catch (const std::bad_alloc& e) {
        cout << "    [ERROR] std::bad_alloc! unordered_map exceeded memory limits.\n";
        cout << "    (This is expected for massive limits on 32-bit processes).\n";
    }

    // --- Benchmark 2: vector ---
    // We allocate cache up to limit. For inputs like 1e6, this takes 4MB (1M * 4 bytes).
    // This memory layout avoids fragmentation and is highly cache-friendly.
    cout << "\n[2] Running vector-based implementation...\n";
    try {
        CollatzVector vector_solver(limit); 
        auto start_vec = high_resolution_clock::now();
        CollatzResult result_vec = vector_solver.find_longest_chain(limit);
        auto end_vec = high_resolution_clock::now();
        duration_vec = duration_cast<milliseconds>(end_vec - start_vec).count();

        cout << "    Longest sequence generated by : " << result_vec.starting_number << "\n";
        cout << "    Total steps                   : " << result_vec.steps << "\n";
        cout << "    Global peak value reached     : " << result_vec.peak_value << "\n";
        cout << "    Execution time                : " << duration_vec << " ms\n";
    } catch (const std::bad_alloc& e) {
        cout << "    [ERROR] std::bad_alloc! vector exceeded available memory.\n";
    }
    
    // --- Benchmark 3: Multi-threaded ---
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    int num_threads = sysinfo.dwNumberOfProcessors > 0 ? sysinfo.dwNumberOfProcessors : 4;
    cout << "\n[3] Running multi-threaded implementation (" << num_threads << " threads)...\n";
    try {
        CollatzMultiThread mt_solver(num_threads);
        auto start_mt = high_resolution_clock::now();
        CollatzResult result_mt = mt_solver.find_longest_chain(limit);
        auto end_mt = high_resolution_clock::now();
        duration_mt = duration_cast<milliseconds>(end_mt - start_mt).count();

        cout << "    Longest sequence generated by : " << result_mt.starting_number << "\n";
        cout << "    Total steps                   : " << result_mt.steps << "\n";
        cout << "    Global peak value reached     : " << result_mt.peak_value << "\n";
        cout << "    Execution time                : " << duration_mt << " ms\n";
    } catch (const std::bad_alloc& e) {
        cout << "    [ERROR] std::bad_alloc! Multi-threaded implementation exceeded available memory.\n";
    }

    // --- Comparison ---
    cout << "\n========================================================\n";
    cout << "Performance Comparison:\n";
    
    if (duration_map >= 0 && duration_vec >= 0) {
        double map_ms = max(1.0, (double)duration_map);
        double vec_ms = max(1.0, (double)duration_vec);
        cout << "Single-threaded Vector was " << fixed << setprecision(2) 
             << (map_ms / vec_ms) << "x faster than Map approach.\n";
    }
    if (duration_vec >= 0 && duration_mt >= 0) {
        double vec_ms = max(1.0, (double)duration_vec);
        double mt_ms = max(1.0, (double)duration_mt);
        cout << "Multi-threaded Vector was  " << fixed << setprecision(2) 
             << (vec_ms / mt_ms) << "x faster than Single-threaded Vector.\n";
    }
    if (duration_map >= 0 && duration_mt >= 0) {
        double map_ms = max(1.0, (double)duration_map);
        double mt_ms = max(1.0, (double)duration_mt);
        cout << "Multi-threaded Vector was  " << fixed << setprecision(2) 
             << (map_ms / mt_ms) << "x faster than Map approach.\n";
    }
    cout << "========================================================\n";
}

int main(int argc, char* argv[]) {
    // Enable fast I/O
    ios::sync_with_stdio(false);
    cin.tie(NULL);

    long long limit = 1000000; // Default limit: 1,000,000

    // Optional: Allow limit to be specified via command-line arguments
    if (argc > 1) {
        try {
            limit = stoll(argv[1]);
        } catch (const exception& e) {
            cerr << "Invalid argument. Using default limit: 1,000,000\n";
            limit = 1000000;
        }
    }

    // 1. Run original optimization benchmarks
    run_benchmark(limit);

    // 2. Run Reverse Collatz Tree generation
    // 25 is a sensible default depth that executes in a few seconds and shows exponential growth
    int max_depth = 25; 
    ReverseTreeAnalyzer::analyze(max_depth);
    
    // 3. Run Compressed Odd-to-Odd Map analysis
    OddToOddAnalyzer::analyze(limit);

    return 0;
}
