# processLogs.py - Script to extract algorithm performance and power usage
#     from log files
#
# Author: Aaron Keesing

import os.path
from glob import glob
import pandas as pd
import numpy as np

TICKS_PER_SECOND = 128
VOLTAGE = 3
POWER_CPU = 1.800*VOLTAGE      # mW
POWER_LPM = 0.0545*VOLTAGE     # mW
POWER_TRANSMIT = 17.7*VOLTAGE  # mW
POWER_LISTEN = 20.0*VOLTAGE    # mW

DATASETS = ('Banana2', 'Noise1', 'Noise2', 'Noise3', 'Noise4', 'Noise5', 'HIWS', 'HITEMP', 'StBernard')
EPOCHS = {'Banana': 460, 'Banana2': 500, 'Noise1': 500, 'Noise2': 500, 'Noise3': 500, 'Noise4': 500, 'Noise5': 500,
          'HIWS': 575, 'HITEMP': 1151, 'StBernard': 719}

def cmCalculator(d, trueD, total):
    tp = len(trueD.intersection(d))
    fp = len(d) - tp
    fn = len(trueD) - tp
    tn = total - tp - fp - fn
    if len(trueD) > 0:
        tpr = tp/(tp + fn)
    else:
        tpr = 0
    if len(d) > 0:
        if tp > 0:
            precision = tp/(tp + fp)
            F1Score = 2*precision*tpr/(precision + tpr)
        else:
            precision = F1Score = 0
    else:
        precision = F1Score = 0
    return (tp, fp, fn, tn, tpr, precision, F1Score)

tests = pd.read_csv('logs/tests.txt', sep=' ')
df_all = {}
for dataset in DATASETS:
    ranges = {1: {}}
    with open(f'data/truth/{dataset}.txt') as f:
        i = 1
        for line in f:
            line = line.strip()
            if len(line) == 0:
                continue
            if line.startswith('#'):
                i = int(line[1:])
                if i not in ranges:
                    ranges[i] = {}
                continue
            nums, tp = line.split()
            nums = nums.split(',')
            for num in nums:
                if '-' in num:
                    a, b = num.split('-')
                    a = int(a)
                    b = int(b)
                    ranges[i][(a, b)] = tp
                else:
                    a = int(num)
                    ranges[i][(a,)] = tp

    filenames = [f for f in glob(f'logs/{dataset}/**', recursive=True)
                 if os.path.basename(f).isnumeric()]
    rows = []
    for filename in filenames:
        ID = int(os.path.basename(filename))
        splits = filename.split(os.path.sep)
        testID = int(splits[-2][4:])
        if not tests['TestID'].isin([testID]).any():
            continue
        testType = splits[-3]
        powerMetrics = []
        L = []
        G = []
        A = []
        with open(filename) as f:
            for line in f:
                if line.startswith('L'):
                    epochs = line[2:].strip().split()
                    L.extend(epochs)
                elif line.startswith('G'):
                    epochs = line[2:].strip().split()
                    G.extend(epochs)
                elif line.startswith('A'):
                    epochs = line[2:].strip().split()
                    A.extend(epochs)
                else:
                    clock, cpu, lpm, send, listen = map(int, line.strip().split())
                    nonRadioTime = cpu + lpm
                    if nonRadioTime == 0:  # Needed for some initial readings
                        continue
                    clock /= TICKS_PER_SECOND
                    cpu *= POWER_CPU/nonRadioTime
                    lpm *= POWER_LPM/nonRadioTime
                    send *= POWER_TRANSMIT/nonRadioTime
                    listen *= POWER_LISTEN/nonRadioTime
                    powerMetrics.append((clock, cpu, lpm, send, listen))

        L = list(map(int, L))
        G = list(map(int, G))
        A = list(map(int, A))
        U = set(L).union(set(G))
        In = set(L).intersection(set(G))
        if dataset == 'HIWS':
            In = set(L)
        if testType == 'adaptive':
            In = set(A)

        with open(filename + '_processed', 'w') as g:
            print('Power Usage', file=g)
            for clock, cpu, lpm, send, listen in powerMetrics:
                print(clock, cpu, lpm, send, listen, file=g)
            print(file=g)
            print('Anomalies', file=g)
            print(' '.join(map(str, In)), file=g)

        powerMetrics = np.array(powerMetrics)
        meanPower = powerMetrics.mean(0)
        meanPower = meanPower[1:]  # ignore clock

        anomalies = dict((x, set()) for x in ['all', 'Noise', 'LocalCluster', 'Environment'])
        for k in ranges[ID]:
            if len(k) == 2:
                nums = range(k[0], k[1]+1)
            else:
                nums = [k[0]]
            anomalies[ranges[ID][k]].update(nums)
            anomalies['all'].update(nums)

        row = [testID, testType, ID]

        if len(L) > 0 or len(G) > 0:
            row.extend(cmCalculator(In, anomalies['Noise'], EPOCHS[dataset]))
            row.extend(cmCalculator(In, anomalies['LocalCluster'], EPOCHS[dataset]))
            row.extend(cmCalculator(In, anomalies['Environment'], EPOCHS[dataset]))
            row.extend(cmCalculator(In, anomalies['all'], EPOCHS[dataset]))
        else:
            row.extend(cmCalculator(A, anomalies['Noise'], EPOCHS[dataset]))
            row.extend(cmCalculator(A, anomalies['LocalCluster'], EPOCHS[dataset]))
            row.extend(cmCalculator(A, anomalies['Environment'], EPOCHS[dataset]))
            row.extend(cmCalculator(A, anomalies['all'], EPOCHS[dataset]))
        row.extend(meanPower)
        rows.append(row)
    df = pd.DataFrame(rows, columns=('TestID', 'Test', 'NodeID',
                                     'NTP', 'NFP', 'NFN', 'NTN', 'Ntpr', 'Nprec', 'NF1Score',
                                     'LCTP', 'LCFP', 'LCFN', 'LCTN', 'LCtpr', 'LCprec', 'LCF1Score',
                                     'ETP', 'EFP', 'EFN', 'ETN', 'Etpr', 'Eprec', 'EF1Score',
                                     'ATP', 'AFP', 'AFN', 'ATN', 'Atpr', 'Aprec', 'AF1Score',
                                     'CPU', 'LPM', 'Tx', 'Listen'))
    df = df.astype({'TestID': int})
    df = pd.merge(tests, df, on=['TestID', 'Test'])
    df = df.set_index('TestID')
    df = df.sort_values(['Test', 'NodeID'])
    df.to_excel(f'logs/{dataset}/results.xlsx', 'Data')
    df_all[dataset] = df
df = pd.concat(df_all, join='inner', names=['Dataset'])
df = df.reset_index()
df.to_excel('logs/all.xlsx', 'Data')

df = df.groupby(['Dataset', 'Algorithm', 'M', 'n', 'nu', 'sigma', 'TestID']) \
    .median() \
    .reindex(columns=('Ntpr', 'Nprec', 'NF1Score',
                      'LCtpr', 'LCprec', 'LCF1Score',
                      'Etpr', 'Eprec', 'EF1Score',
                      'Atpr', 'Aprec', 'AF1Score'))
df.round(2) \
    .drop(columns=['Ntpr', 'Nprec', 'NF1Score', 'LCtpr', 'LCprec', 'LCF1Score', 'Etpr', 'Eprec', 'EF1Score']) \
    .to_latex('logs/summary.tex', bold_rows=True)

colHeaders = [['Noise']*3 + ['LocalCluster']*3 + ['Environment']*3 + ['All']*3,
              ['Ntpr', 'Nprec', 'NF1Score', 'LCtpr', 'LCprec', 'LCF1Score', 'Etpr', 'Eprec', 'EF1Score', 'Atpr', 'Aprec', 'AF1Score']]
df.columns = pd.MultiIndex.from_tuples(zip(*colHeaders), names=['Type', 'Metric'])
df.to_excel('logs/summary.xlsx', 'Data')

for dataset in DATASETS:
    df.loc[dataset, :].to_excel(f'logs/{dataset}/summary.xlsx')
    df.loc[dataset, :].round(2).to_latex(f'logs/{dataset}/summary.tex', bold_rows=True)
    for algorithm in ['Adaptive', 'Periodic', 'Original']:
        df.loc[(dataset, algorithm), :].round(2).to_latex(f'logs/{dataset}/summary_{algorithm}.tex', bold_rows=True)
