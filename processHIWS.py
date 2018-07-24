# processHeron.py - Process the Heron Island data
#
# Author: Aaron Keesing

import pandas as pd

filepath = '~/Documents/Windows/Uni Stuff/2018/Dissertation/Datasets/Heron_Island_Data.csv'

df = pd.read_csv(filepath, parse_dates=['TIME'], infer_datetime_format=True)
df = df.drop(columns=['VALUES_quality_control', 'url', 'platform_code', 'channel_id',
                      'LATITUDE', 'LONGITUDE', 'geom', 'NOMINAL_DEPTH', 'FID'])
df = df.dropna(0)

# HIWS has all variables except TEMP, the other nodes have TEMP at various depths
df = df[df['site_code'] == 'HIWS'].drop(columns='site_code')
df = df[df['VARNAME'].isin(('WSPD_10min', 'WDIR_10min', 'AIRT', 'ATMP', 'RELH',
                            'RAIN_AMOUNT'))]
df = df[(pd.Timestamp('2009-03-07') < df['TIME']) & (df['TIME'] < pd.Timestamp('2009-03-11'))]
df = df.set_index(['TIME', 'VARNAME']).sort_index()
df = df.groupby(level=['TIME', 'VARNAME']).mean().unstack('VARNAME')
df.columns = df.columns.get_level_values('VARNAME')
df.index = list(range(df.index.size))
df.index.name = 'Epoch'

# Convert to ints
df.AIRT *= 10
df.ATMP *= 10
df.RAIN_AMOUNT *= 10
df.RELH *= 10
df.WDIR_10min *= 100
df.WSPD_10min *= 100
df = df.astype(int)

df.to_csv(f'./data/HIWS', sep=' ', header=False, index=True)
