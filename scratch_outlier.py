def analyze_outlier(n):
    steps = 0
    trajectory = [n]
    while n != 1:
        if n % 2 == 0:
            n = n // 2
        else:
            n = 3 * n + 1
        trajectory.append(n)
        steps += 1
    
    odds = [x for x in trajectory if x % 2 == 1]
    evens = [x for x in trajectory if x % 2 == 0]
    
    print(f"Number: {trajectory[0]}")
    print(f"Total Steps: {steps}")
    print(f"Max Value: {max(trajectory)}")
    print(f"Odd steps: {len(odds)}, Even steps: {len(evens)}")
    print(f"Ratio of Even/Odd: {len(evens) / len(odds):.4f} (Expected ~2.0 if random drift)")

analyze_outlier(837799)
