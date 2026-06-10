import pandas as pd
import matplotlib.pyplot as plt
import os
import glob

def visualize_tree():
    files = glob.glob('data/csv/reverse_tree_*.csv')
    if not files:
        print("No reverse tree data found.")
        return
    
    file = max(files, key=os.path.getctime)
    df = pd.read_csv(file)
    
    plt.figure(figsize=(10, 6))
    plt.plot(df['depth'], df['nodes'], marker='o', linestyle='-', color='b')
    plt.yscale('log')
    plt.title('Reverse Collatz Tree Growth (Log Scale)')
    plt.xlabel('Depth')
    plt.ylabel('Number of Nodes')
    plt.grid(True, which="both", ls="--", alpha=0.5)
    plt.savefig('data/tree_growth.png')
    print("Saved data/tree_growth.png")

def visualize_stats():
    files = glob.glob('data/csv/statistical_v2_limit_*.csv')
    if not files:
        print("No stats data found.")
        return
    
    file = max(files, key=os.path.getctime)
    df = pd.read_csv(file)
    
    plt.figure(figsize=(10, 6))
    plt.bar(df['v_2'], df['measured_prob'], alpha=0.6, label='Measured Probability', color='blue')
    plt.plot(df['v_2'], df['theoretical_prob'], color='red', marker='x', linestyle='dashed', label='Theoretical (1/2^k)')
    
    plt.title('Distribution of v_2 (Powers of 2 Divided)')
    plt.xlabel('v_2')
    plt.ylabel('Probability')
    plt.legend()
    plt.grid(axis='y', linestyle='--', alpha=0.7)
    plt.savefig('data/v2_distribution.png')
    print("Saved data/v2_distribution.png")

if __name__ == '__main__':
    if not os.path.exists('data'):
        os.makedirs('data')
    print("Generating visualizations from exported CSVs...")
    visualize_tree()
    visualize_stats()
    print("Done!")
