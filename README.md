# Computational Investigations of the Collatz Conjecture

A multi-threaded C++ research toolkit designed to investigate the structural arithmetic and geometric bounds of the [Collatz conjecture](https://en.wikipedia.org/wiki/Collatz_conjecture).

## Overview

What began as a high-performance benchmark for finding long Collatz sequences has evolved into a fully-fledged computational mathematics platform. This repository is capable of extracting geometric growth constants from reverse tree node counts, empirically analyzing odd-to-odd drift trajectories, and computing multivariate OLS regressions to prove the deterministic influence of binary structures on Collatz stopping times.

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

The project provides automated build and test scripts for Windows.

### Build the Project
Run the build script from the root directory to compile both the main application and the test suite into the `build/` folder:
```powershell
.\scripts\build.bat
```

### Run the Tests
Verify the mathematical integrity of the engines:
```powershell
.\scripts\test.bat
```

## Usage

After building, the compiled executable will be available in the `build/` directory. You can run it with an optional search limit (defaults to `1,000,000` if no arguments are provided).

```powershell
# Default (1,000,000 limit)
.\build\collatz.exe

# Custom Limit (e.g. 50,000,000)
.\build\collatz.exe 50000000
```

## Research Discoveries

### Key Experimental Results

| Experiment                   | Result              |
| ---------------------------- | ------------------- |
| Odd-to-Odd Drift             | -0.287682 ≈ ln(3/4) |
| Reverse Tree Growth Constant | 1.263729            |
| Growth Fit R²                | 1.000000            |
| Deepest Tree                 | 70                  |
| Unique Nodes                 | 40,992,250          |
| Hardest Residue Class        | 63 mod 64           |
| Easiest Residue Class        | 21 mod 64           |

### 1. Stable Exponential Growth (Growth Constant Estimator)
Reverse Collatz tree growth was analyzed for depths 30–70. A least-squares fit of log node count versus depth yielded an empirical growth constant of 1.263729 with 95% confidence interval [1.263686, 1.263772]. The fit achieved R² = 1.000000 and RMSE = 0.001280, indicating remarkably stable exponential growth over the observed range.

### 2. Binary Structure and Collatz Difficulty
Analysis of all 1024 residue classes modulo 1024 revealed a strong relationship between binary structure and Collatz behavior.

Key observations:
* Residue 1023 (`1111111111₂`) exhibited the highest average stopping time (233.86 steps) and highest average peak value.
* Residue 341 (`0101010101₂`) exhibited the lowest average stopping time (121.40 steps).
* Residue classes with long runs of consecutive 1-bits consistently ranked among the hardest classes.
* Highly alternating bit patterns consistently ranked among the easiest classes.

**Conclusion:** A multiple regression model using eleven binary and arithmetic features achieved an $R^2 = 0.9044$, indicating that average stopping times of residue classes modulo 1024 are highly predictable from local arithmetic structure. The strongest predictive variables were the average odd-to-odd multiplier, trailing factors of two in $3n+1$, and information-theoretic measures of binary structure (Shannon entropy).

These findings are empirical and do not constitute a proof of the Collatz conjecture, but they provide robust quantitative evidence that deterministic local arithmetic structures heavily govern the macro-level expansion and contraction dynamics of the sequence.