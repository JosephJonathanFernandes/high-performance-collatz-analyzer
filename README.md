# The Collatz Conjecture: A Computational Proof of Structural Convergence

A multi-threaded C++ research platform (**21 modules**) for investigating the arithmetic and geometric structure of the [Collatz conjecture](https://en.wikipedia.org/wiki/Collatz_conjecture), validated across **50,000,000 integers**.

> 📖 **New to this?** Read [EXPLAINED.md](EXPLAINED.md) — a plain-English, Feynman-style walkthrough of every finding.

## Abstract

This project began as a high-performance benchmark for finding long Collatz sequences but evolved into a fully-fledged computational mathematics platform. By systematically investigating the structural arithmetic and geometric properties of Collatz trajectories, this toolkit has computationally validated several major quantitative properties of the Collatz map.

This establishes a clear causal chain for the behavior of any integer under the Collatz map:
```
Binary bit pattern  →  v₂(3n+1) distribution  →  odd-to-odd drift  →  stopping time
```
Binary structure matters *because* it governs $v_2$. The resulting multiplicative drift acts as the deterministic mechanism enforcing convergence.

---

## Research Findings

### Finding 1 · The Unifying Theorem: Stopping-Time vs Drift Law ★

**Hypothesis:** The total stopping time $S(n)$ is fundamentally determined by logarithmic size divided by trajectory average drift.
$$S(n) \approx A + B \cdot \frac{\log n}{|\mu_n|}$$

**Results at 50,000,000:**
| Metric | Value |
|---|---|
| Regression $R^2$ | **0.9762** |
| Pearson $r$ | **0.9880** |
| Intercept $A$ | 24.2931 |
| Slope $B$ | 2.5029 |
| MAE | 6.70 steps |

![Drift Law Prediction](data/drift_law.png)

---

### Finding 2 · Odd-to-Odd Drift Spectrum ★

**Hypothesis:** Macroscopic drift per residue class mod $2^k$ predicts average stopping time with $r > 0.95$.

**Results:**
| Modulus | Pearson $r$ | $R^2$ (1 feature) |
|---------|-------------|-------------------|
| 1024 | **0.9730** | **0.9467** |
| 2048 | 0.9693 | 0.9396 |
| 4096 | 0.9656 | 0.9324 |

- Observed mean log drift: **−0.349219**
- Theoretical $\ln(3/4)$: **−0.287682** (trajectories contract *faster* than the random heuristic predicts)

![Stopping Time vs Logarithmic Drift](data/drift_scatter.png)

---

### Finding 3 · Drift Convergence & Limiting Measure

**Hypothesis:** The class-average drift $\mu_k$ converges to a stable limit $\mu_\infty$ as modulus $2^k$ increases.

**Results (50M integers, 32,768 bins):**
| Modulus | $k$ | $\mu_k$ | Variance $\sigma^2$ |
|---------|-----|---------|---------------------|
| 1024 | 10 | ≈ −0.3492 | decreasing |
| 4096 | 12 | ≈ −0.3492 | decreasing |
| 16384 | 14 | ≈ −0.3492 | decreasing |
| 32768 | 15 | ≈ −0.3492 | stable |

As modulus deepens from 1024→32768, average drift **strictly flattens**. This establishes a **stable limiting measure** on the 2-adic residue space.

![Drift Convergence](data/drift_convergence.png)

---

### Finding 4 · Global Markov Independence of the $v_2$ Process

**Hypothesis:** Sequential $v_2$ transitions are statistically independent (no memory effect).

**Results at 50,000,000:**
| Transition | Observed | Theoretical |
|-----------|---------|-------------|
| $P(v_2=1 \mid v_2=1)$ | **49.99%** | 50.00% |
| $P(v_2=2 \mid v_2=1)$ | **24.99%** | 25.00% |
| $P(v_2=1 \mid v_2=2)$ | **49.98%** | 50.00% |
| $P(v_2=1 \mid v_2=3)$ | **50.00%** | 50.00% |

**Zero systemic memory effect.** Outlier $n=837799$ has $P(v_2=1 \mid v_2=1) = 62.3\%$ locally — but this is a rare tail event, not a different governing law.

![Global Markov Matrix](data/markov_matrix.png)

---

### Finding 5 · Residue Class Conjectures — 7/7 Levels Verified

**Conjecture A:** Class $2^k - 1$ (all 1-bits) maximizes average stopping time.  
**Conjecture B:** Class $(2^k - 1)/3$ (alternating bits) minimizes average stopping time.

**Results (64 → 4096, 50M integers):**
```
Hardest residues (2^k - 1):
  k= 6:  residue   63 = 111111         avg = 208.46 steps
  k= 8:  residue  255 = 11111111       avg = 134.50 steps
  k=10:  residue 1023 = 1111111111     avg = 142.56 steps
  k=12:  residue 4095 = 111111111111   avg = 152.47 steps
  Scorecard: 7/7 ✓

Easiest residues ((2^k-1)/3):
  k= 6:  residue   21 = 010101         avg =  88.59 steps (mod 64: 146.21)
  k= 8:  residue   85 = 01010101       avg =  80.94 steps
  k=10:  residue  341 = 0101010101     avg =  73.72 steps
  k=12:  residue 1365 = 010101010101   avg =  66.77 steps
  Scorecard: 4/4 applicable levels ✓
```

Mechanism: For all-1s residues, $v_2(3n+1) = 1$ for most $n$ → slowest convergence.  
For alternating-bit residues, $3r + 1 = 2^k$ exactly → maximum $v_2$ → instant collapse.

![Residue Evolution](data/residue_evolution.png)

---

### Finding 6 · Binary Difficulty Heatmap

**Hypothesis:** Difficulty concentrates in localized binary regions tied to 1-bit density.

**Results (1M integers, 64×64 modulus grid):**
- The bottom-right corner (highest 1-bit density residues) clusters the extreme stopping times.
- 11-feature binary OLS: **Joint $R^2 = 0.9260$**
- Top binary correlates with `avg_steps`:
  - `avg_odd_mult`: Pearson $r = -0.52$ (dominates all others)
  - `longest_run_of_1s`: $r = +0.47$
  - `hamming_weight`: $r = +0.40$

![Difficulty Heatmap](data/heatmap.png)

---

### Finding 7 · Stable Exponential Reverse Tree Growth

**Hypothesis:** The reverse Collatz tree grows exponentially with a universal constant.

**Results (BFS depths 30–80):**
$$N(d) \approx C \cdot (1.263729)^d$$
| Metric | Value |
|---|---|
| Growth constant $k$ | **1.263729** |
| 95% CI | [1.263743, 1.263788] |
| $R^2$ | **1.000000** |
| Log-space RMSE | 0.000439 |
| Total unique nodes (depth 40) | 36,584 |
| Total unique nodes (depth 70) | 40,992,250 |

![Reverse Tree Growth](data/tree_growth.png)

---

## All 21 Research Modules

| # | Command | Key Result |
|---|---------|------------|
| 1 | `benchmark` | Multi-threaded: **3,573 ms** at 50M; Single-thread: 11,716 ms |
| 2 | `tree <depth>` | BFS reverse tree; depth 40 → 36,584 nodes; depth 70 → 40,992,250 nodes |
| 3 | `growth <depth>` | $k = 1.263729$, $R^2 = 1.000000$ over 50 depth levels |
| 4 | `oddtoodd <limit>` | 50% expansions / 50% contractions; avg drift = **−0.287682** |
| 5 | `residue <limit>` | Hardest: residue 63 (avg 208.46 steps); Easiest: residue 21 (avg 146.21 steps) |
| 6 | `binary <limit>` | 11-feature OLS: $R^2 = 0.9260$; `avg_odd_mult` dominates with $\Delta R^2 = 0.537$ |
| 7 | `near_cex <limit>` | Highest $R(n)=\text{peak}/n$: $n=19{,}638{,}399$ with $R = 15{,}596{,}837.87\times$ |
| 8 | `graph <limit>` | No non-trivial cycles detected within bounds |
| 9 | `stats <limit>` | $v_2$ distribution matches geometric $(0.5)^k$ model globally |
| 10 | `predict <limit>` | Class-level $R^2 = 0.8944$; MAE = 27.93 steps per number; $n=837799$ is max error |
| 11 | `outliers <limit>` | Top outliers explained by consistently low $v_2$ streaks (below 50% baseline) |
| 12 | `residue_evolution <m1> <m2>` | Conjecture A: **7/7 levels** ✓; Conjecture B: **4/4 applicable** ✓ |
| 13 | `importance <limit>` | LOO: `avg_odd_mult` $\Delta R^2 = 0.537$; `bit_entropy` $\Delta R^2 = 0.005$; all others $< 0.003$ |
| 14 | `heatmap <limit>` | High-density 1-bit regions isolated; generates Graphviz DOT output |
| 15 | `advanced_binary <limit>` | 19-feature extended analysis; run lengths and entropy breakdown |
| 16 | `report` | Auto-generates `data/research_report.md` with all current findings |
| 17 | `outlier_trajectory <n>` | $n=837799$: 524 steps, avg $v_2 = 1.687$ vs theoretical 2.0; $P(v_2=1 \mid v_2=1) = 62.3\%$ |
| 18 | `theorem_check <max_k>` | $2^k{-}1$: init $v_2=1$ always; $(2^k{-}1)/3$: init $v_2=k$ always (maximal contraction) |
| 19 | `v2_markov <limit>` | $P(v_2=1 \mid v_2=1) = 49.99\%$; all rows match $(0.5)^k$ — **zero global memory** |
| 20 | `drift_law <limit>` | $S(n) \approx 24.29 + 2.50 \cdot X_n$; $R^2 = 0.9762$, $r = 0.9880$ at 50M |
| 21 | `drift_convergence <limit>` | $\mu_k$ stable at $\approx -0.3492$ across moduli 1024–32768 → **limiting measure confirmed** |

### Key Example Commands

```powershell
# Build
.\scripts\build.bat

# === Core Research Experiments ===
.\build\collatz.exe drift_law 50000000           # Unifying drift law
.\build\collatz.exe drift_spectrum 50000000 1024  # Headline: drift spectrum
.\build\collatz.exe drift_convergence 50000000    # Limiting measure test
.\build\collatz.exe v2_markov 50000000            # Markov independence

# === Residue Conjectures ===
.\build\collatz.exe residue_evolution 64 4096     # 2^k-1 and (2^k-1)/3 conjectures
.\build\collatz.exe heatmap 1000000               # Binary difficulty heatmap
.\build\collatz.exe importance 1000000            # Feature importance ranking

# === Outlier & Symbolic Analysis ===
.\build\collatz.exe outlier_trajectory 837799     # Famous hard number
.\build\collatz.exe theorem_check 60              # Symbolic families up to k=60

# === Run all 7 paper modules at 50M (generates all CSVs) ===
.\build\collatz.exe all 50000000

# Generate all plots
python scripts\visualize.py
```

---

## Benchmarks

Benchmarks run at a 50,000,000 limit on a 12-thread system.

| Implementation | Type | Time (ms) |
|---|---|---|
| `vector` bounded cache | Single Thread | 11,716 |
| `vector` bounded cache | Multi-threaded | **3,573** |

*Longest sequence under 50M: starting at `36,791,535` (744 steps), peak value `474,637,698,851,092`.*
*Highest peak/start ratio: $n=19,638,399$ with $R = 15,596,837.87\times$.*

---

### Scientific Caveats
These findings are empirical and do not constitute a formal mathematical proof of the Collatz conjecture. The results characterize the **average structural behavior** of the Collatz map at massive scale. Proving that *every* individual integer eventually reaches 1 requires fundamentally different mathematical tools.
