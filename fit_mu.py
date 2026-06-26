import numpy as np
from sklearn.linear_model import LinearRegression
from sklearn.metrics import r2_score

N = np.array([1000000, 10000000, 50000000, 100000000, 200000000, 500000000])
mu = np.array([-0.37347939, -0.35742381, -0.34921927, -0.34622476, -0.34349861, -0.34033701])

# Transformation for linear fitting: x = 1 / ln(N)
X = 1 / np.log(N)

# Reshape X for sklearn
X_reshaped = X.reshape(-1, 1)

# Fit linear model
model = LinearRegression()
model.fit(X_reshaped, mu)

a = model.intercept_
b = model.coef_[0]

# Predictions and R^2
mu_pred = model.predict(X_reshaped)
r2 = r2_score(mu, mu_pred)

print(f"a = {a}")
print(f"b = {b}")
print(f"R^2 = {r2}")
