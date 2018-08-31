# processBanana.py - Process the synthetic banana dataset
#
# Author: Aaron Keesing

import pandas as pd
import numpy as np

filepath = '~/Dropbox/Dissertation/data/synth/banana.csv'

df = pd.read_csv(filepath)

# 365 in the upper right cluster
clusterAnomalies = list(df.index[(df.Class == 2) & (df.V2 > 1.2 - 0.4*df.V1)])
noiseAnomalies = list(df.index[(df.Class == 2) & (df.V2 <= 1.2 - 0.4*df.V1)])
realData = list(df.index[df.Class == 1])
df.loc[clusterAnomalies, 'Class'] = 3

np.random.shuffle(clusterAnomalies)
np.random.shuffle(noiseAnomalies)

cluster1L = clusterAnomalies[0:30]
cluster2L = clusterAnomalies[30:60]
cluster3L = clusterAnomalies[60:90]
cluster1G = clusterAnomalies[90:120]
cluster2G = clusterAnomalies[120:150]
cluster3G = clusterAnomalies[150:180]

node1Points = realData[0:400]
node2Points = realData[400:800]
node3Points = realData[800:1200]

noise = np.random.choice(noiseAnomalies, 180, False)
randIndices = np.random.choice(400, 90, False)
print(1)
for idx in sorted(randIndices[0:30]):
    print(idx, 'G,NoiseW')
print(2)
for idx in sorted(randIndices[30:60]):
    print(idx, 'G,NoiseW')
print(3)
for idx in sorted(randIndices[60:90]):
    print(idx, 'G,NoiseW')

for i in range(30):
    node1Points[randIndices[i]] = noise[i]
    node2Points[randIndices[30+i]] = noise[30+i]
    node3Points[randIndices[60+i]] = noise[60+i]

# 460 data points each
# A cluster of 30 for each node by itself
# A cluster of 30 for all nodes at the same time
# 30 `noise' data points randomly interspersed
node1Points = node1Points[:100] + cluster1L + node1Points[100:370] + cluster1G + node1Points[370:]
node2Points = node2Points[:200] + cluster1L + node2Points[200:370] + cluster1G + node2Points[370:]
node3Points = node3Points[:300] + cluster1L + node3Points[300:370] + cluster1G + node3Points[370:]

node1 = df.loc[node1Points].reset_index(drop=True)
node2 = df.loc[node2Points].reset_index(drop=True)
node3 = df.loc[node3Points].reset_index(drop=True)

node1 = node1.drop(columns='Class')
node2 = node2.drop(columns='Class')
node3 = node3.drop(columns='Class')
node1 *= 1000
node2 *= 1000
node3 *= 1000
node1 = node1.astype(int)
node2 = node2.astype(int)
node3 = node3.astype(int)

node1.to_csv('./data/Banana_1', sep=' ', header=False, index=True)
node2.to_csv('./data/Banana_2', sep=' ', header=False, index=True)
node3.to_csv('./data/Banana_3', sep=' ', header=False, index=True)
