# processIBRL.py - Process the IBRL data
#
# Author: Aaron Keesing
# Date: 07/07/18

import pandas as pd

datapath = '/home/aaron/Documents/Windows/Uni Stuff/2018/Dissertation/Datasets/IBRL_data.txt'
locpath = '/home/aaron/Documents/Windows/Uni Stuff/2018/Dissertation/Datasets/IBRL_mote_locs.txt'
connectivity = '/home/aaron/Documents/Windows/Uni Stuff/2018/Dissertation/Datasets/IBRL_connectivity.txt'

df = pd.read_csv(datapath,
                 sep=' ',
                 names=('Date', 'Time', 'Epoch', 'Id', 'Temp', 'Humidity', 'Light', 'Voltage'),
                 parse_dates=[['Date', 'Time']],
                 infer_datetime_format=True)
df = df[(1 <= df.Id) & (df.Id <= 54)]  # Include only the original sensors
df = df[(df.Epoch >= 120) & (df.Epoch <= 65535)]  # Exclude invalid epochs
df = df.set_index(['Date_Time'])
df = df.sort_index()
df = df.loc[:'2004-03-21 19:06:15']
df = df.drop_duplicates(['Id', 'Epoch'])
df = df.dropna(0)  # Remove any missing data rows

# import matplotlib.pyplot as plt
# plt.figure()
# plt.hist(df.Epoch, bins=60)
# plt.show()

df = df.loc['2004-02-29']
df = df.set_index(['Id', 'Epoch'])
minEpoch = min(x[1] for x in df.index)
maxEpoch = max(x[1] for x in df.index)
df.reindex(pd.MultiIndex.from_product([range(1, 55), range(minEpoch, maxEpoch + 1)], names=['Id', 'Epoch']))
df = df.unstack(0).interpolate(limit_direction='both').unstack(0).unstack(0)
ids = pd.unique(df.index.get_level_values('Id'))
print('Valid ids:')
print(ids)
# Node 5 has only voltage data
# Node 28 doesn't have light data

conn = {ID: {} for ID in ids}
with open(connectivity) as f, open('dgrm.dat', 'w') as o:
    for line in f:
        a, b, p = line.strip().split()
        a, b = int(a), int(b)
        p = float(p)
        if a != 0 and b != 0 and a != b:
            if a in (5, 28) or b in (5, 28):
                continue
            conn[a][b] = p
            print(a, b, p, 1, 0, 0, -20, -100, -10, file=o)
input("Press enter to continue or Ctrl-C to exit.")

coords = []
with open(locpath) as f:
    for line in f:
        coords.append(list(map(float, line.strip().split()[1:])))
print(f'coords = {coords}')
input("Press enter to continue or Ctrl-C to exit.")

# Convert to ints
df.Temp *= 10000
df.Humidity *= 10000
df.Light *= 100
df.Voltage *= 100000
df = df.astype({'Temp': int, 'Humidity': int, 'Light': int, 'Voltage': int})

for ID in ids:
    dfI = df.loc[ID, :]
    dfI.to_csv(f'./data/IBRL_{ID}', sep=' ', header=False, index=True)
