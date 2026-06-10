#pragma once

#include <unordered_map>
#include <vector>
#include "CollatzResult.h"

namespace collatz {

/**
 * @brief Collatz sequence optimizer using std::unordered_map for memoization.
 *
 * Provides unbounded caching capability but suffers from high memory overhead
 * and cache-misses compared to a contiguous array approach.
 */
class CollatzMap {
private:
    std::unordered_map<long long, int> cache;

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
            
            std::vector<long long> path;
            path.reserve(1000); 

            while (current != 1) {
                auto it = cache.find(current);
                if (it != cache.end()) {
                    steps += it->second;
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

} // namespace collatz
