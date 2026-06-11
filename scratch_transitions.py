def analyze_transitions(n):
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
            
    transitions = {}
    for i in range(len(v2_seq) - 1):
        v_curr = v2_seq[i]
        v_next = v2_seq[i+1]
        if v_curr not in transitions:
            transitions[v_curr] = []
        transitions[v_curr].append(v_next)
        
    for k in sorted(transitions.keys()):
        from collections import Counter
        c = Counter(transitions[k])
        total = len(transitions[k])
        print(f"After v2={k} ({total} times):")
        for next_k in sorted(c.keys()):
            print(f"  -> v2={next_k}: {c[next_k]} times ({c[next_k]/total*100:.1f}%)")

analyze_transitions(837799)
