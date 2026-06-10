#pragma once

#include <vector>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <algorithm>
#include "CollatzResult.h"

namespace collatz {

/**
 * @brief Thread arguments payload for Win32 API
 */
struct ThreadData {
    long long start_val;
    long long end_val;
    long long limit;
    CollatzResult result;
};

/**
 * @brief Thread entry point worker function
 */
inline DWORD WINAPI CollatzThreadWorker(LPVOID lpParam) {
    ThreadData* data = (ThreadData*)lpParam;
    
    // Thread-local cache: cap size to prevent massive memory usage per thread
    long long local_cache_limit = (std::min)(data->limit, 5000000LL);  
    std::vector<int> local_cache(local_cache_limit + 1, -1);
    local_cache[1] = 0;

    long long local_longest_start = data->start_val;
    int local_max_steps = 0;
    long long local_global_peak = 1;

    for (long long i = data->start_val; i <= data->end_val; ++i) {
        long long current = i;
        int steps = 0;
        long long peak = current;
        
        std::vector<long long> path;
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

/**
 * @brief Multi-threaded Collatz optimizer using Win32 threads.
 *
 * Divides the search limit into chunks and executes in parallel. Uses localized
 * caches to eliminate lock contention completely.
 */
class CollatzMultiThread {
private:
    int num_threads;

public:
    explicit CollatzMultiThread(int threads) : num_threads(threads) {}

    CollatzResult find_longest_chain(long long limit) {
        if (limit < num_threads) num_threads = static_cast<int>(limit);
        
        long long chunk_size = limit / num_threads;
        
        std::vector<HANDLE> threads(num_threads);
        std::vector<ThreadData> thread_data(num_threads);

        for (int t = 0; t < num_threads; ++t) {
            thread_data[t].start_val = t * chunk_size + 1;
            thread_data[t].end_val = (t == num_threads - 1) ? limit : (thread_data[t].start_val + chunk_size - 1);
            thread_data[t].limit = limit;

            threads[t] = CreateThread(
                NULL, 0, CollatzThreadWorker, &thread_data[t], 0, NULL);
        }

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

} // namespace collatz
