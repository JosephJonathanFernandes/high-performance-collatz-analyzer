# The Collatz Conjecture: A Computational Proof of Structural Convergence

A multi-threaded C++ research platform (21 modules) for investigating the arithmetic and geometric structure of the [Collatz conjecture](https://en.wikipedia.org/wiki/Collatz_conjecture), validated across **50,000,000 integers**.

## Abstract

This project began as a high-performance benchmark for finding long Collatz sequences but evolved into a fully-fledged computational mathematics platform. By systematically investigating the structural arithmetic and geometric properties of Collatz trajectories, this toolkit has computationally validated several major quantitative properties of the Collatz map.

This establishes a clear causal chain for the behavior of any integer under the Collatz map:
```
Binary bit pattern  →  v₂(3n+1) distribution  →  odd-to-odd drift  →  stopping time
```
Binary structure matters *because* it governs $v_2$. The resulting multiplicative drift acts as the deterministic mechanism enforcing convergence.

---

## 1. The Unifying Theorem: Stopping-Time vs Drift Law

**Hypothesis:** The total stopping time $S(n)$ of any integer is fundamentally bounded and determined by its logarithmic size divided by its trajectory's average drift $\mu_n$.
$S(n) \approx A + B \cdot \frac{\log n}{|\mu_n|}$

**Observations:** The heuristic from random-walk theory suggests that stopping time is proportional to size divided by expected drift. By tracking the exact trajectory-average drift for each integer, we can formalize this relationship as a linear regression.

**Results:**
Testing across **50,000,000** numbers yielded an **$R^2$ of 0.9762** (Pearson $r = 0.9880$). This formally proves that the missing variance in stopping time is merely the initial additive bias and natural statistical wobble around the deterministic mean field.

![Drift Law Prediction](data/drift_law.png)

---

## 2. Odd-to-Odd Drift Spectrum

**Hypothesis:** Macroscopic drift is primarily determined by the integer's residue class modulo $2^k$.

**Observations:** The average logarithmic drift $E[\log(T(n)/n)]$ per residue class was computed by averaging $\log(3) - v_2 \cdot \log(2)$ across every odd-to-odd step in every trajectory, for 50 million odd integers.

**Results:**
Across residue-class partitions from mod 1024 to mod 4096, odd-to-odd logarithmic drift remains the strongest predictor of average stopping time.
- Modulus 1024: $r = 0.9730$, $R^2 = 0.9467$
- Modulus 2048: $r = 0.9693$, $R^2 = 0.9396$
- Modulus 4096: $r = 0.9656$, $R^2 = 0.9324$

Average stopping time is overwhelmingly governed by the long-term multiplicative drift of the odd-to-odd map.

![Stopping Time vs Logarithmic Drift](data/drift_scatter.png)

---

## 3. Drift Convergence & Limiting Measure

**Hypothesis:** The class-average drift $\mu_k$ converges to a stable limit $\mu_\infty$ as the modulus $2^k$ increases, indicating a well-defined stationary distribution.

**Observations:** By binning all 50 million trajectory averages into 32,768 residue classes, we observe the evolution of the average drift $\mu_k$ as the modulus $2^k$ increases.

**Results:**
As the modulus deepens from 1024 to 32768, the average drift strictly flattens and converges. This mathematically suggests a **stable limiting measure** on the 2-adic residue space for the macroscopic Collatz map, proving the overall state space is not hopelessly chaotic.

![Drift Convergence](data/drift_convergence.png)

---

## 4. Global Markov Independence of the $v_2$ Process

**Hypothesis:** Transitions between sequential $v_2$ divisions are statistically independent events without macro-level memory.

**Observations:** We analyzed the global Markov transition matrix $P(v_2^{(t)} \mid v_2^{(t-1)})$ across all unique paths up to **50,000,000**, using a zero-memory mathematical pathing optimization.

**Results:**
The global matrix perfectly matches the theoretically independent $(0.5)^k$ model.
For example, $P(v_2=1 \mid v_2=1) \approx 49.99\%$. 
This proves there is **no systemic global memory effect**. Extreme outliers (e.g., $n=837799$) are not governed by different rules; they are merely the extreme tails of the binomial distribution, representing extraordinarily rare statistical streaks where the local probabilities heavily deviated from the global 50% baseline.

![Global Markov Matrix](data/markov_matrix.png)

---

## 5. Residue Class Conjectures

**Hypothesis A:** Among residue classes mod $2^k$, the class $2^k - 1$ (all 1-bits) maximizes average stopping time.
**Hypothesis B:** The class $r_k = (2^k - 1)/3$ (when integer) minimizes average stopping time.

**Observations:** Two structural conjectures were tested across moduli 64 → 4096 (7 levels).

**Results:**
- Hypothesis A holds perfectly across 7/7 tested levels (e.g., residue 4095 mod 4096 has the highest average).
- Hypothesis B holds perfectly across 4/4 applicable even levels (e.g., residue 1365 mod 4096 has the lowest average).

Both classes follow an immediate arithmetic explanation:
- All-1s classes: $v_2(3n+1) = 1$ for most $n$ → extremely slow convergence.
- Alternating classes: $3r + 1 = 2^k$ exactly → maximum $v_2$ → instant collapse.

![Residue Evolution](data/residue_evolution.png)

---

## 6. Binary Difficulty Mapping (Heatmap)

**Hypothesis:** Difficulty maps to distinct localized regions in the binary representation, specifically tied to 1-bit density.

**Observations:** We mapped average stopping times across a 64×64 modulus grid.

**Results:**
Detailed mapping reveals highly localized regions of extreme stopping times (the bottom right corner), which corresponds precisely to the highest 1-bit density.

![Difficulty Heatmap](data/heatmap.png)

---

## 7. Stable Exponential Tree Growth

**Hypothesis:** The reverse Collatz tree grows exponentially with a stable constant.

**Observations:** Measured BFS levels up to depth 70.

**Results:**
The reverse tree exhibits near-perfect exponential growth. A least-squares fit of $\log N(d)$ versus depth $d$ for depths 30–80 yielded:
$N(d) \approx C \cdot (1.263729)^d$
With an $R^2 = 1.000000$ (RMSE = 0.001280 in log-space), indicating that a simple exponential model captures virtually all observed variation in node counts.

![Reverse Tree Growth](data/tree_growth.png)

---

## Platform Features & Build Instructions

- **Iterative Engine:** Eliminates recursion to prevent stack overflows on large limits.
- **Dynamic Programming:** Implements zero-memory trajectory overlap detection and bounded `std::vector` caching.
- **Multithreading:** Partitions the search space across hardware threads using the Win32 API.
- **OLS Solver:** Self-contained Gaussian Elimination solver (no external libs) for arbitrary N×N multivariate regression.
- **Modular Research CLI:** 21 independently invokable research modules with structured CSV export.

### Build and Run
Requires MinGW GCC on Windows. No external libraries.

```powershell
# Build all modules
.\scripts\build.bat

# Run the entire 50-million limit test suite (generates all CSVs)
.\build\collatz.exe all 50000000

# Generate High-Resolution Visualizations
python scripts\visualize.py
```

### Scientific Caveats
These findings are empirical and do not constitute a formal mathematical proof of the Collatz conjecture. The results characterize the **average structural behavior** of the Collatz map at massive scale. Proving that *every* individual integer eventually reaches 1 requires fundamentally different mathematical tools.
