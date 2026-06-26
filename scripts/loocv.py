import numpy as np
from sklearn.linear_model import LinearRegression

L_1B = np.log(1000000000)
pred_1B = -0.288321 - 0.739311/L_1B - 6.037356/(L_1B**2)
actual_1B = pred_1B - 0.0000326

N = np.array([
    1000000, 
    10000000, 
    50000000, 
    100000000, 
    200000000, 
    300000000, 
    500000000, 
    1000000000, 
    2000000000
])

mu = np.array([
    -0.37347939, 
    -0.35742381, 
    -0.34921927, 
    -0.34622476, 
    -0.34349861, 
    -0.34202109, 
    -0.34033701, 
    actual_1B, 
    -0.33611781
])

# Features: 1/ln(N) and 1/ln(N)^2
L = np.log(N)
X = np.column_stack((1 / L, 1 / (L**2)))

errors = []

for i in range(len(N)):
    # Create mask for leaving one out
    mask = np.ones(len(N), dtype=bool)
    mask[i] = False
    
    X_train = X[mask]
    mu_train = mu[mask]
    X_test = X[i].reshape(1, -1)
    mu_test = mu[i]
    
    # Fit model on remaining 8 points
    model = LinearRegression()
    model.fit(X_train, mu_train)
    
    # Predict omitted point
    pred = model.predict(X_test)[0]
    
    # Calculate absolute error
    abs_error = abs(mu_test - pred)
    errors.append(abs_error)

# Calculate MAE and max error
mae = np.mean(errors)
max_error = np.max(errors)

print(f"Mean absolute error: {mae:.6e}")
print(f"Maximum absolute error: {max_error:.6e}")
