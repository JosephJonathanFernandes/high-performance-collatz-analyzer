# Computational Investigations of the Collatz Conjecture

A multi-threaded C++ research platform (16 modules) for investigating the arithmetic and geometric structure of the [Collatz conjecture](https://en.wikipedia.org/wiki/Collatz_conjecture).

## Main Finding

> Across all 1024 residue classes modulo 1024 and 50 million analyzed integers, average stopping time exhibits a **Pearson correlation of 0.9730** with average odd-to-odd logarithmic drift $E[\log(T(n)/n)]$. A **single drift feature** explains **94.67% of stopping-time variance**, outperforming substantially more complex binary-feature models.

```
avg_log_drift alone    →  R² = 0.9467   (1 feature)
11 binary features     →  R² = 0.9044
4 binary features      →  R² = 0.3200
```

This establishes the causal chain:

```
Binary bit pattern  →  v₂(3n+1)  →  odd-to-odd drift  →  stopping time
```

Binary structure matters *because* it governs $v_2$. The drift is the mechanism.

---

## Overview

What began as a high-performance benchmark for finding long Collatz sequences has evolved into a fully-fledged computational mathematics platform. The platform systematically investigates the structural arithmetic and geometric properties of Collatz trajectories across 50 million integers, using reverse tree exploration, residue class analysis, OLS regression modelling, and drift spectrum analysis.

## Platform Features

- **Iterative Engine:** Eliminates recursion to prevent stack overflows on large limits.
- **Dynamic Programming:** Implements memoization via both `std::unordered_map` and a contiguous bounded `std::vector`.
- **Path Compression:** Back-propagates sequence lengths to intermediate nodes encountered during computation.
- **Bitwise Optimization:** Uses bitwise shifts (`>> 1`) and masks (`& 1`) for all division and modulo operations.
- **Multithreading:** Partitions the search space across hardware threads using the Win32 API with thread-local caches to eliminate lock contention.
- **OLS Solver:** Self-contained Gaussian Elimination solver (no external libs) for arbitrary N×N multivariate regression.
- **Modular Research CLI:** 16 independently invokable research modules with structured CSV export.

## Research Modules

| Module | Command | Description |
|--------|---------|-------------|
| 1 | `benchmark` | Extreme performance benchmark (multi-threaded) |
| 2 | `tree` | Reverse Collatz Tree (BFS, depth up to 70) |
| 3 | `growth` | Least-squares exponential fit of tree node counts |
| 4 | `oddtoodd` | Compressed Odd-to-Odd Map analyzer |
| 5 | `residue` | Residue class difficulty analyzer |
| 6 | `binary` | 11-feature binary pattern correlation + OLS |
| 7 | `near_cex` | Near-counterexample detector |
| 8 | `graph` | Sequence graph + cycle detection |
| 9 | `stats` | v₂ probability distribution verifier |
| 10 | `predict` | Per-number stopping time predictor (MAE/MSE/R²) |
| 11 | `outliers` | Outlier discovery (numbers least explained by model) |
| 12 | `residue_evolution` | Track hardest/easiest residue class as modulus grows |
| 13 | `importance` | LOO + Permutation feature importance ranking |
| 14 | `heatmap` | Difficulty heatmap + Graphviz DOT coloring |
| 15 | `advanced_binary` | 19-feature advanced binary structure analysis |
| 16 | `report` | Auto-generate `data/research_report.md` |
| **★** | **`drift_spectrum`** | **Odd-to-Odd Drift Spectrum — the headline result** |

## Build Instructions

```powershell
# Build all modules
.\scripts\build.bat

# Run test suite
.\scripts\test.bat
```

Requires MinGW GCC on Windows. No external libraries.

## Usage

```powershell
# Run the headline experiment
.\build\collatz.exe drift_spectrum 50000000 1024

# Feature importance ranking
.\build\collatz.exe importance 1000000

# Track residue conjecture stability
.\build\collatz.exe residue_evolution 64 4096

# Auto-generate research report
.\build\collatz.exe report
```

## Benchmarks

Benchmarks run at a 50,000,000 limit on a 12-thread system.

| Implementation | Type | Time (ms) |
|---|---|---|
| `vector` bounded cache | Single Thread | 11,716 |
| `vector` bounded cache | Multi-threaded | 3,573 |

*Longest sequence under 50M: starting at `36,791,535` (744 steps), peak value `474,637,698,851,092`.*

## Research Discoveries

### Key Experimental Results

| Experiment | Result |
|---|---|
| **Drift Spectrum r (mod 1024)** | **0.9730** |
| **Drift Spectrum r (mod 2048)** | **0.9693** |
| **Drift Spectrum r (mod 4096)** | **0.9656** |
| Drift Spectrum R² (mod 1024) | 0.9467 (1 feature) |
| Observed mean log drift | −0.349219 |
| Theoretical ln(3/4) | −0.287682 |
| 11-Feature Regression R² | 0.9044 |
| Reverse Tree Growth Constant | 1.263729 |
| Growth Fit R² | 1.000000 |
| Deepest Tree Explored | depth 70 |
| Unique Tree Nodes | 40,992,250 |
| Hardest Residue Conjecture | **7/7 levels** ✓ |
| Easiest Residue Conjecture | 4/4 applicable levels ✓ |

---

### 1. Odd-to-Odd Drift Spectrum (★ Headline Result)

The average logarithmic drift $E[\log(T(n)/n)]$ per residue class was computed by averaging $\log(3) - v_2 \cdot \log(2)$ across every odd-to-odd step in every trajectory, for 50 million odd integers.

**Drift stability across moduli:**

| Modulus | Pearson r | R² (1 feature) |
|---------|-----------|----------------|
| 1024 | **0.9730** | **0.9467** |
| 2048 | 0.9693 | 0.9396 |
| 4096 | 0.9656 | 0.9324 |

The correlation decreases only slightly as resolution doubles. The relationship is not an artifact of the 1024-class partition.

**Summary statement:**
> Across residue-class partitions from mod 1024 to mod 4096, odd-to-odd logarithmic drift remains the strongest predictor of average stopping time, with Pearson correlation consistently above 0.96.

**Interpretation:** Average stopping time is overwhelmingly governed by the long-term multiplicative drift of the odd-to-odd map. Binary features (run lengths, entropy, Hamming weight) matter primarily because they influence the distribution of $v_2(3n+1)$, which directly determines that drift.

**Note on drift magnitude:** The observed mean drift (−0.349219) is more negative than the naive geometric-distribution prediction of $\ln(3/4) = -0.287682$. Real Collatz trajectories contract slightly *faster* than the uniform random heuristic would suggest — a systematic deviation worth further investigation.

---

### 2. Residue Class Conjectures — Stability Verified

Two structural conjectures were tested across moduli 64 → 4096 (7 levels).

**Conjecture A:** Among residue classes mod $2^k$, the class $2^k - 1$ (all 1-bits) maximizes average stopping time.

```
Scorecard: 7 / 7  ✓  (holds at every tested level)

k= 6: residue   63 = 111111         avg=126.73
k= 7: residue  127 = 1111111        avg=130.61
k= 8: residue  255 = 11111111       avg=134.50
k= 9: residue  511 = 111111111      avg=138.39
k=10: residue 1023 = 1111111111     avg=142.56
k=11: residue 2047 = 11111111111    avg=147.27
k=12: residue 4095 = 111111111111   avg=152.47
```

**Conjecture B:** The class $r_k = (2^k - 1)/3$ (when integer) minimizes average stopping time.

```
Applicable at even k only (integer condition):

k= 6: residue   21 = 010101         avg= 88.59  ✓
k= 8: residue   85 = 01010101       avg= 80.94  ✓
k=10: residue  341 = 0101010101     avg= 73.72  ✓
k=12: residue 1365 = 010101010101   avg= 66.77  ✓

Scorecard: 4 / 4 applicable levels  ✓
```

At odd k, $(2^k - 1)/3$ is not an integer — those cases are inapplicable, not failures.

Both classes follow an immediate arithmetic explanation:
- All-1s classes: $v_2(3n+1) = 1$ for most $n$ → slow convergence
- Alternating classes: $3r + 1 = 2^k$ exactly → maximum $v_2$ → instant collapse

---

### 2. Stable Exponential Tree Growth

Reverse Collatz tree growth was analyzed for depths 30–70. A least-squares fit of $\log N(d)$ versus depth $d$ yielded:

```
N(d) ≈ C · (1.263729)^d
k    = 1.263729   (95% CI: [1.263686, 1.263772])
R²   = 1.000000
RMSE = 0.001280 (log-space)
```

An R² of 1.000000 over 40 depth levels indicates that a simple exponential model captures virtually all observed variation in node counts.

---

### 3. Binary Structure and Residue Classes

Analysis of all 1024 residue classes modulo 1024 over 50 million integers revealed:

- Residue **1023** (`1111111111₂`) — highest average stopping time.
- Residue **341** (`0101010101₂`) — lowest average stopping time.
- Residue classes with long trailing runs of 1-bits consistently rank hardest.
- Classes satisfying $3r + 1 = 2^k$ consistently rank easiest.

**Feature Importance (LOO ΔR²):**
```
avg_odd_mult      :  ΔR² = 0.537  ← dominates
bit_entropy       :  ΔR² = 0.005
hamming_weight    :  ΔR² = 0.002
trailing_ones     :  ΔR² = 0.001
(all others)      :  ΔR² < 0.001
```

`avg_odd_mult` alone accounts for more than half the model's explanatory power.

---

### Scientific Caveats

These findings are empirical and do not constitute a proof of the Collatz conjecture. The results characterize **average behavior of residue classes**, not the behavior of every individual integer. Proving that every integer eventually reaches 1 requires fundamentally different mathematical tools.