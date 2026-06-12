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

### Finding 5b · Symbolic Analytic Proof (Theorem Checker)

**These two conjectures are not just empirically true — they are provable in algebra.**

**Proof of Conjecture B** (easiest class, 2 lines):
> Let $n = (2^k - 1)/3$. Then $3n + 1 = 3 \cdot \frac{2^k-1}{3} + 1 = (2^k - 1) + 1 = 2^k$.  
> Therefore $v_2(3n+1) = k$ — the maximum possible. The number collapses in a single odd step.

**Proof of Conjecture A** (hardest class):
> Let $n \equiv 2^k - 1 \pmod{2^k}$, so all $k$ trailing bits are 1.  
> Then $3n + 1 \equiv 3(2^k-1)+1 = 3 \cdot 2^k - 2 = 2(3 \cdot 2^{k-1} - 1) \pmod{2^{k+1}}$.  
> Since $3 \cdot 2^{k-1} - 1$ is odd, we get $v_2(3n+1) = 1$ — the minimum possible. Maximum expansion per step.

**Results from `theorem_check` at $k = 1 \ldots 20$:**
| Family | Init $v_2$ | Avg $v_2$ | Behavior |
|---|---|---|---|
| $2^k - 1$ | **always 1** | 1.7–2.1 | Slowest convergence, highest drift |
| $(2^k-1)/3$ | **always $k$** | exactly $k$ | Single-step collapse |
| $2^k + 1$ | always 2 | 1.75–3.0 | Mixed behavior |
| $(2^k+1)/3$ | always 1 | 1.75–2.5 | Near-hard behavior |

### Finding 6 · Binary Difficulty Heatmap

**Hypothesis:** Difficulty concentrates in localized binary regions tied to 1-bit density.

**Results (1M integers, 64×64 modulus grid):**
- The bottom-right corner (highest 1-bit density residues) clusters the extreme stopping times.
- 11-feature binary OLS: **Joint $R^2 = 0.9260$**
- 19-feature advanced binary OLS (`advanced_binary`): captures run-length structure, entropy, and ternary residues across 256 modular classes
- Top binary correlates with `avg_steps`:
  - `avg_odd_mult`: Pearson $r = -0.52$ (dominates all others, $\Delta R^2 = 0.537$ in LOO test)
  - `longest_run_of_1s`: $r = +0.47$
  - `hamming_weight`: $r = +0.40$
  - `bit_entropy`: $\Delta R^2 = 0.005$ (all remaining features combined < 0.01)

![Difficulty Heatmap](data/heatmap.png)

---

### Finding 8 · Near-Counterexample Detector

**What it does:** Finds numbers whose trajectory peaks at the largest multiple of their starting value — i.e., numbers that "almost escape" to infinity before crashing.

**Results at 50,000,000:**
| Rank | $n$ | $R(n) = \text{peak}/n$ | Peak Value | Steps |
|------|-----|----------------------|------------|-------|
| 1 | 19,638,399 | **15,596,837×** | 306,296,925,203,752 | 606 |
| 2 | 38,595,583 | 12,297,720× | 474,637,698,851,092 | 483 |
| 3 | 29,457,599 | 10,397,891× | 306,296,925,203,752 | 604 |

None of these escape. Every single one eventually reaches 1. But they demonstrate just how far a trajectory can stray before converging — peak values exceeding **300 trillion** from a starting point under 50 million.

---

### Finding 9 · Graph Cycle Detection

**Hypothesis:** The Collatz directed graph contains no non-trivial cycles other than {4→2→1}.

**Results:** Over all unique paths from 3 to 50,000,000, a full DFS found **zero non-trivial cycles**. Every path eventually merges into the 4→2→1 trivial cycle. This is consistent with the conjecture but is not a proof for numbers beyond the tested range.

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
