import numpy as np

def rbf(x, y, s=0.5):
    return np.exp(-np.linalg.norm(x - y)/(s*s))

data = np.fromfile('./data/HIWS', sep=' ').reshape((-1, 7))
data[:, 1:5] /= 10
data[:, 5:] /= 100
data = data[:, 1:]

K = np.zeros((30, 30))
for i in range(30):
    for j in range(30):
        K[i, j] = rbf(data[i], data[j])
one = np.ones_like(K) / 30
Kc = K - one.dot(K) - K.dot(one) + one.dot(K).dot(one)
print(sorted(np.diag(Kc)))
