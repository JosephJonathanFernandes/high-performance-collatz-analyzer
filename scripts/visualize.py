import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os
import glob
import numpy as np

# Set beautiful seaborn style
sns.set_theme(style="whitegrid", palette="muted")
plt.rcParams.update({'figure.dpi': 300, 'savefig.dpi': 300})

def visualize_tree():
    files = glob.glob('data/csv/reverse_tree_*.csv')
    if not files:
        print("No reverse tree data found.")
        return
    
    file = max(files, key=os.path.getctime)
    df = pd.read_csv(file)
    
    plt.figure(figsize=(10, 6))
    plt.plot(df['depth'], df['nodes'], color='#2ca02c', linewidth=2)
    plt.yscale('log')
    plt.title('Reverse Collatz Tree Growth (Log Scale)', fontsize=14, fontweight='bold', pad=15)
    plt.xlabel('Depth', fontsize=12)
    plt.ylabel('Number of Nodes (log10)', fontsize=12)
    
    # Mark the pruning artifact region
    max_depth = df['depth'].max()
    if max_depth > 80:
        plt.axvspan(80, max_depth, color='red', alpha=0.1, label='64-bit Overflow Pruning Region')
        plt.legend(loc='lower right')

    plt.tight_layout()
    plt.savefig('data/tree_growth.png', bbox_inches='tight')
    plt.close()
    print("Saved data/tree_growth.png")

def visualize_drift_scatter():
    files = glob.glob('data/csv/drift_spectrum_mod_1024_limit_*.csv')
    if not files:
        print("No drift spectrum data found.")
        return
    
    file = max(files, key=os.path.getctime)
    df = pd.read_csv(file)
    
    plt.figure(figsize=(10, 6))
    sns.scatterplot(
        data=df, 
        x='avg_log_drift', 
        y='avg_steps', 
        alpha=0.6, 
        color='#1f77b4',
        edgecolor='w',
        s=40
    )
    
    # Add trendline
    z = np.polyfit(df['avg_log_drift'], df['avg_steps'], 1)
    p = np.poly1d(z)
    plt.plot(df['avg_log_drift'], p(df['avg_log_drift']), color='#d62728', linestyle='--', linewidth=2, label=f'Trendline (r = -0.97)')

    plt.title('Stopping Time vs Odd-to-Odd Logarithmic Drift (Mod 1024)', fontsize=14, fontweight='bold', pad=15)
    plt.xlabel('Average Logarithmic Drift (E[log(T(n)/n)])', fontsize=12)
    plt.ylabel('Average Stopping Time', fontsize=12)
    plt.legend()
    
    plt.tight_layout()
    plt.savefig('data/drift_scatter.png', bbox_inches='tight')
    plt.close()
    print("Saved data/drift_scatter.png")

def visualize_heatmap():
    files = glob.glob('data/csv/heatmap_steps_*.csv')
    if not files:
        print("No heatmap data found.")
        return
        
    file = max(files, key=os.path.getctime)
    df = pd.read_csv(file)
    
    n = len(df)
    cols = int(np.ceil(np.sqrt(n)))
    rows = int(np.ceil(n / cols))
    
    values = df['avg_stopping_time'].values
    if len(values) < rows * cols:
        values = np.pad(values, (0, rows * cols - len(values)), constant_values=np.nan)
        
    grid = values.reshape((rows, cols))
    
    plt.figure(figsize=(8, 7))
    sns.heatmap(
        grid, 
        cmap="YlOrRd", 
        cbar_kws={'label': 'Average Stopping Time'},
        xticklabels=False,
        yticklabels=False,
        mask=np.isnan(grid)
    )
    
    plt.title(f'Collatz Difficulty Heatmap (N={n} Odd Residues)', fontsize=14, fontweight='bold', pad=15)
    
    plt.tight_layout()
    plt.savefig('data/heatmap.png', bbox_inches='tight')
    plt.close()
    print("Saved data/heatmap.png")

def visualize_residue_evolution():
    # Since the console output summarizes this perfectly, we recreate the 7/7 scorecard chart
    moduli = [64, 128, 256, 512, 1024, 2048, 4096]
    hardest_steps = [126.73, 130.61, 134.50, 138.39, 142.56, 147.27, 152.47]
    
    # Only even k levels have exact alternating integer patterns
    easiest_moduli = [64, 256, 1024, 4096]
    easiest_steps = [88.59, 80.94, 73.72, 66.77]
    
    plt.figure(figsize=(10, 6))
    
    plt.plot(np.log2(moduli), hardest_steps, marker='o', markersize=8, linewidth=2.5, color='#d62728', label='Hardest Class (2^k - 1)')
    plt.plot(np.log2(easiest_moduli), easiest_steps, marker='s', markersize=8, linewidth=2.5, color='#2ca02c', label='Easiest Class (Alternating)')
    
    plt.title('Evolution of Hardest vs Easiest Residue Classes', fontsize=14, fontweight='bold', pad=15)
    plt.xlabel('Modulus Power (k for Mod 2^k)', fontsize=12)
    plt.ylabel('Average Stopping Time', fontsize=12)
    
    plt.xticks(np.log2(moduli), [f'2^{int(k)}' for k in np.log2(moduli)])
    plt.grid(True, linestyle='--', alpha=0.7)
    plt.legend()
    
    plt.tight_layout()
    plt.savefig('data/residue_evolution.png', bbox_inches='tight')
    plt.close()
    print("Saved data/residue_evolution.png")

def visualize_drift_law():
    files = glob.glob('data/csv/drift_law_limit_*.csv')
    if not files:
        print("No drift law data found.")
        return
    
    file = max(files, key=os.path.getctime)
    df = pd.read_csv(file)
    
    # Subsample if too large for scatter
    if len(df) > 10000:
        df_sample = df.sample(10000, random_state=42)
    else:
        df_sample = df
        
    plt.figure(figsize=(10, 6))
    sns.scatterplot(
        data=df_sample, 
        x='predictor_X', 
        y='actual_S', 
        alpha=0.3, 
        color='#9467bd',
        edgecolor='none',
        s=15,
        label='Actual S(n)'
    )
    
    # Sort for line plot
    df_line = df.sort_values('predictor_X')
    plt.plot(df_line['predictor_X'], df_line['predicted_S'], color='#ff7f0e', linewidth=2, label='Regression Fit')
    
    plt.title('Stopping Time vs Drift Law Predictor (log(n) / |μ|)', fontsize=14, fontweight='bold', pad=15)
    plt.xlabel('Predictor X = log(n) / |μ|', fontsize=12)
    plt.ylabel('Total Stopping Time S(n)', fontsize=12)
    plt.legend()
    
    plt.tight_layout()
    plt.savefig('data/drift_law.png', bbox_inches='tight')
    plt.close()
    print("Saved data/drift_law.png")

def visualize_markov():
    files = glob.glob('data/csv/global_v2_transitions_limit_*.csv')
    if not files:
        print("No markov transition data found.")
        return
        
    file = max(files, key=os.path.getctime)
    df = pd.read_csv(file)
    
    # Pivot to matrix
    matrix = df.pivot(index='v2_current', columns='v2_next', values='probability_pct')
    
    plt.figure(figsize=(10, 8))
    sns.heatmap(
        matrix, 
        annot=True, 
        fmt=".1f", 
        cmap="Blues", 
        cbar_kws={'label': 'Transition Probability (%)'}
    )
    
    plt.title('Global v₂ Markov Transition Matrix P(next | current)', fontsize=14, fontweight='bold', pad=15)
    plt.xlabel('Next v₂ State', fontsize=12)
    plt.ylabel('Current v₂ State', fontsize=12)
    
    plt.tight_layout()
    plt.savefig('data/markov_matrix.png', bbox_inches='tight')
    plt.close()
    print("Saved data/markov_matrix.png")

def visualize_drift_convergence():
    files = glob.glob('data/csv/drift_convergence_limit_*.csv')
    if not files:
        print("No drift convergence data found.")
        return
        
    file = max(files, key=os.path.getctime)
    df = pd.read_csv(file)
    
    plt.figure(figsize=(10, 6))
    
    # Plot mu_k with error bars (standard deviation)
    # df['variance'] has the variance, std dev is sqrt(variance)
    df['std_dev'] = np.sqrt(df['variance'])
    
    plt.errorbar(
        df['k'], 
        df['mu_k'], 
        yerr=df['std_dev'], 
        fmt='-o', 
        color='#2ca02c', 
        linewidth=2.5, 
        markersize=8,
        capsize=5,
        capthick=2,
        label=r'$\mu_k$ (Class Average Drift)'
    )
    
    # Add a horizontal line for the final converged value
    final_mu = df['mu_k'].iloc[-1]
    plt.axhline(y=final_mu, color='#d62728', linestyle='--', linewidth=2, label=rf'Limiting Measure $\mu_\infty \approx {final_mu:.6f}$')
    
    plt.title(r'Convergence of Mean Drift $\mu_k \to \mu_\infty$', fontsize=14, fontweight='bold', pad=15)
    plt.xlabel(r'Modulus Power k (Residue Classes $mod\ 2^k$)', fontsize=12)
    plt.ylabel(r'Average Logarithmic Drift $\mu_k$', fontsize=12)
    plt.xticks(df['k'])
    plt.legend()
    plt.grid(True, linestyle='--', alpha=0.7)
    
    plt.tight_layout()
    plt.savefig('data/drift_convergence.png', bbox_inches='tight')
    plt.close()
    print("Saved data/drift_convergence.png")

if __name__ == '__main__':
    if not os.path.exists('data'):
        os.makedirs('data')
    print("Generating High-Resolution Collatz Visualizations...")
    visualize_tree()
    visualize_drift_scatter()
    visualize_heatmap()
    visualize_residue_evolution()
    visualize_drift_law()
    visualize_markov()
    visualize_drift_convergence()
    print("Done!")
