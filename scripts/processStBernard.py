# processStBernard.py - Process the Grand-St Bernard data
#
# Author: Aaron Keesing

import pandas as pd
import os.path
import sys

filepath = '~/Documents/Windows/Uni Stuff/2018/Dissertation/Datasets/stbernard/cluster/'
_filepath = '~/Documents/Windows/Uni Stuff/2018/Dissertation/Datasets/stbernard/cluster/stbernard-meteo-25.txt'

# Map original IDs to {1..5}
idMap = {25: 1, 28: 2, 29: 3, 31: 4, 32: 5}

def parseDates(_y, _m, _d, _H, _M, _S):
    dates = []
    for y, m, d, H, M, S in zip(_y, _m, _d, _H, _M, _S):
        dates.append(pd.Timestamp(f'{y}-{m}-{d} {H}:{M}:{S}'))
    return dates

df = pd.read_csv(_filepath,
                 sep=' ',
                 names=('ID', 'Year', 'Month', 'Day', 'Hour', 'Minute', 'Second', 'UnixTime', 'Ambient', 'Surface',
                        'Radiation', 'Humidity', 'SoilMoisture', 'Watermark', 'Rain', 'WindSpeed', 'WindDir'),
                 parse_dates={'Time': ['Year', 'Month', 'Day', 'Hour', 'Minute', 'Second']},
                 date_parser=parseDates)
df = df.drop(columns=['ID', 'UnixTime'])
df = df.interpolate(limit_direction='both')
df = df.dropna(0)
df = df.set_index('Time')

for oID, ID in idMap.items():
    df = pd.read_csv(os.path.join(filepath, f'stbernard-meteo-{oID}.txt'),
                     sep=' ',
                     names=('ID', 'Year', 'Month', 'Day', 'Hour', 'Minute', 'Second', 'UnixTime', 'Ambient', 'Surface',
                            'Radiation', 'Humidity', 'SoilMoisture', 'Watermark', 'Rain', 'WindSpeed', 'WindDir'),
                     parse_dates={'Time': ['Year', 'Month', 'Day', 'Hour', 'Minute', 'Second']},
                     date_parser=parseDates)
    df = df.drop(columns=['ID', 'UnixTime'])
    df = df.interpolate(limit_direction='both')
    df = df.dropna(0)
    df = df.set_index('Time')

    # One day's worth should be decent
    df = df.loc['2007-09-30']
    df.index = list(range(df.index.size))
    df.index.name = 'Epoch'

    if len(sys.argv) > 1 and sys.argv[1] == 'normalise':
        for col in df.columns:
            mx = max(df.loc[:, col])
            mn = min(df.loc[:, col])
            if mx == mn:
                df.loc[:, col] = 0
            else:
                df.loc[:, col] = (df.loc[:, col] - mn)/(mx - mn)

    # Convert to ints
    df *= 1000
    df = df.astype(int)

    df.to_csv(f'./data/StBernard_{ID}', sep=' ', header=False, index=True)
