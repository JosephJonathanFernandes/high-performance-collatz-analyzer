import numpy as np
from sklearn.linear_model import LinearRegression
from sklearn.metrics import r2_score

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

# Features: 1/(N^0.1)
X = (1 / (N**0.1)).reshape(-1, 1)

# Fit linear model
model = LinearRegression()
model.fit(X, mu)

a = model.intercept_
b = model.coef_[0]

# Predictions and R^2
mu_pred = model.predict(X)
r2 = r2_score(mu, mu_pred)

print(f"a = {a:.6f}")
print(f"b = {b:.6f}")
print(f"R^2 = {r2:.6f}")
