# high-performance-collatz-analyzer

A multi-threaded C++ toolkit for computing and researching the Collatz Conjecture at extreme limits.

## Overview

This repository contains a C++ program designed to compute the starting number under a given limit that produces the longest [Collatz sequence](https://en.wikipedia.org/wiki/Collatz_conjecture). It utilizes dynamic programming and multithreading to optimize the search space.

In addition to sequence benchmarking, the project includes modules for generating the reverse directed graph and analyzing the compressed odd-to-odd map.

## Features

- **Iterative Engine:** Eliminates recursion to prevent stack overflows on large limits.
- **Dynamic Programming:** Implements memoization using both `std::unordered_map` and a contiguous bounded `std::vector` to evaluate memory-speed trade-offs.
- **Path Compression:** Back-propagates sequence lengths to intermediate numbers encountered during computation, pruning the search tree.
- **Bitwise Optimization:** Uses bitwise shifts (`>> 1`) and masks (`& 1`) for division and modulo operations.
- **Multithreading:** Partitions the search space across hardware threads using the Win32 API. Employs thread-local caches to eliminate mutex lock contention.

## Research Modules

- **Reverse Tree Generator (`ReverseTree.h`):** Uses Breadth-First Search (BFS) to construct the directed graph of numbers flowing into `1`, measuring the branching factor of the Collatz tree at a given depth.
- **Odd-to-Odd Map Analyzer (`OddToOdd.h`):** Evaluates the compressed map `T(n) = (3n+1) / 2^(v_2)`. Tracks expansions vs. contractions, maximum expansion ratio, and the distribution of powers of 2.

## Benchmarks

Benchmarks were run for a search limit of `50,000,000` on a 12-thread system.

| Implementation | Type | Time (ms) | Notes |
|---|---|---|---|
| `unordered_map` | Single Thread | N/A | Exceeds 2GB memory limit on 32-bit processes |
| `vector` bounded cache | Single Thread | 11,716 | |
| `vector` bounded cache | Multi-threaded | 3,573 | 3.28x performance scaling |

*Note: The longest sequence under 50,000,000 is produced by `36,791,535` (744 steps), with a peak value of `474,637,698,851,092`.*

## Build Instructions

The project can be built using `g++` directly or via CMake.

### Using g++
```bash
g++ main.cpp -o collatz.exe -O3
```

### Using CMake
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## Usage

Run the executable. A default limit of `1,000,000` is used if no arguments are provided.

```bash
# Default (1,000,000 limit)
./collatz.exe

# Custom Limit (e.g. 50,000,000)
./collatz.exe 50000000
```