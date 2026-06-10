# Architecture Overview

This project is built for maximum computational efficiency while maintaining standard software engineering modularity.

## Core Concepts

### 1. Zero Recursion
The standard Collatz calculation relies on recursion. At high numerical limits, this guarantees a stack overflow. Our implementations utilize `while` loops and back-propagation arrays to eliminate stack growth entirely.

### 2. Path Compression
When evaluating the Collatz sequence for `N`, we encounter dozens or hundreds of intermediate numbers. Instead of discarding these, we push them to an intermediate `std::vector` cache and back-propagate the final step count to all of them. This acts as a massive heuristic pruning mechanism, meaning future loops will find cache hits almost instantly.

### 3. Contiguous Memory (Vector) vs Sparse Memory (Map)
- **`CollatzMap`**: Uses `std::unordered_map`. Can cache any arbitrary large number encountered, but is severely penalized by hash collisions and memory fragmentation overhead. Hits memory limits around 50M on 32-bit machines.
- **`CollatzVector`**: Uses a contiguous `std::vector`. Only caches numbers up to the absolute `limit` defined. The lookup `O(1)` time has practically zero CPU overhead compared to map hashing, yielding massive speedups.

### 4. Lock-Free Multithreading
Traditional parallel memoization requires a global cache secured by a `std::mutex`. Under load, this causes horrific lock contention, actually making multithreading *slower* than single-threaded execution. 
Our `CollatzMultiThread` assigns an entirely localized `vector` cache to each individual thread. Threads compute chunks of the input independently without ever communicating, achieving near 100% linear performance scaling based on core count.
