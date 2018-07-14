# processHeron.py - Process the Heron Island data
#
# Author: Aaron Keesing
# Date: 14/07/18

import pandas as pd

filepath = '~/Dropbox/Dissertation/data/Heron_Island_Data.csv'

df = pd.read_csv(filepath, parse_dates=['TIME'], infer_datetime_format=True)
sitelocs = df.groupby(['site_code',
                       'LATITUDE',
                       'LONGITUDE']).size().reset_index((1, 2)).drop(columns=0)
df = df.drop(columns=['VALUES_quality_control', 'url', 'platform_code',
                      'channel_id', 'LATITUDE', 'LONGITUDE', 'geom',
                      'NOMINAL_DEPTH', 'FID'])
df = df.dropna(0)

# HIWS has all variables except TEMP
# The other nodes have TEMP at various depths
df = df[df['site_code'] != 'HIWS'].drop(columns='VARNAME')
df = df[(pd.Timestamp('2009-03-07') < df['TIME']) &
        (df['TIME'] < pd.Timestamp('2009-03-11'))]
df = df.set_index(['TIME', 'site_code']).sort_index().unstack('site_code')
df.columns = df.columns.get_level_values('site_code')
df.index = list(range(df.index.size))
df.index.name = 'Epoch'
df = df.interpolate()

# Convert to ints
df *= 1000
df = df.astype(int)

for i, node in enumerate(df.columns):
    df.loc[:, node].to_csv(f'./data/HITEMP_{i+1}',
                           sep=' ',
                           header=False,
                           index=True)
