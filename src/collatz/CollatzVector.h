#pragma once

#include <vector>
#include "CollatzResult.h"

namespace collatz {

/**
 * @brief Collatz sequence optimizer using std::vector for bounded memoization.
 *
 * Utilizes a contiguous memory layout to maximize CPU cache hits and eliminate
 * hash function overhead, vastly outperforming map-based solutions.
 */
class CollatzVector {
private:
    std::vector<int> cache;
    long long cache_limit;

public:
    explicit CollatzVector(long long max_limit) : cache_limit(max_limit) {
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
            
            std::vector<long long> path;
            path.reserve(1000);

            while (current != 1) {
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

            int remaining_steps = steps;
            for (long long node : path) {
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

} // namespace collatz
