import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

Ndata = 500
Nnoise = 50
Ncluster = 10

for ID in [1, 2, 3]:
    x = np.random.uniform(0.3, 0.8, size=Ndata)
    y = 5*(x - 0.6)**2 + 0.2
    x += np.random.normal(0, 0.03, size=Ndata)
    y += np.random.normal(0, 0.04, size=Ndata)
    np.clip(x, 0, 1)
    np.clip(y, 0, 1)
    realData = np.c_[x, y]

    noise = np.random.multivariate_normal([0.6, 0.7], [[0.002, 0], [0, 0.002]], size=Nnoise)
    local = np.random.multivariate_normal([0.6, 0.7], [[0.002, 0], [0, 0.002]], size=Ncluster)
    environment = np.random.multivariate_normal([0.6, 0.7], [[0.002, 0], [0, 0.002]], size=Ncluster)

    plt.figure()
    plt.title(f'Sensor node {ID}')
    plt.scatter(*realData.T, s=10, c='black')
    # plt.scatter(*noise.T, s=10, c='green')
    plt.scatter(*local.T, s=10, c='green')
    plt.axis([0, 1, 0, 1])
    plt.xlabel('x')
    plt.ylabel('y')
    plt.draw()

    data = realData
    # indices = np.random.choice(list(range(ID*100)) + list(range(ID*100+Ncluster, 400)) + list(range(430, 500)), Nnoise)
    # print(f'#{ID}')
    # for idx in indices:
    #     print(idx, 'Noise')
    # print()
    # data[indices] = noise
    # data[ID*100:ID*100+Ncluster] = local
    data[400:400+Ncluster] = environment

    df = pd.DataFrame(data)
    df *= 1000
    df = df.astype(int)
    # df.to_csv(f'data/Noise_{ID}', sep=' ', index=True, header=False)
plt.show()
