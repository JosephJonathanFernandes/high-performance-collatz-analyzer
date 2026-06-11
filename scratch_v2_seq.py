def analyze_trajectory_v2(n):
    trajectory = []
    v2_seq = []
    
    current = n
    while current != 1:
        if current % 2 == 1:
            trajectory.append(current)
            current = 3 * current + 1
            v2 = 0
            while current % 2 == 0:
                current = current // 2
                v2 += 1
            v2_seq.append(v2)
        else:
            current = current // 2
            
    print(f"Number: {n}")
    print(f"Odd steps: {len(v2_seq)}")
    print(f"Average v2: {sum(v2_seq)/len(v2_seq):.4f}")
    
    from collections import Counter
    c = Counter(v2_seq)
    print("v2 distribution:")
    for k in sorted(c.keys()):
        print(f"  v2={k}: {c[k]} times ({c[k]/len(v2_seq)*100:.1f}%)")
        
    print("v2 sequence:")
    print(", ".join(map(str, v2_seq)))

analyze_trajectory_v2(837799)
