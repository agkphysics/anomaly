# processLogs.py - Script to extract algorithm performance and power usage
#     from log files
#
# Author: Aaron Keesing

import os.path
from glob import glob
import pandas as pd

TICKS_PER_SECOND = 4096
VOLTAGE = 3
POWER_CPU = 1.800*VOLTAGE      # mW
POWER_LPM = 0.0545*VOLTAGE     # mW
POWER_TRANSMIT = 17.7*VOLTAGE  # mW
POWER_LISTEN = 20.0*VOLTAGE    # mW

DATASETS = ('Banana', 'HIWS', 'HITEMP', 'StBernard')
totalEpochs = {'Banana': 460, 'HIWS': 575, 'HITEMP': 1151, 'StBernard': 719}

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
            tp = tp.split(',')
            if len(tp) == 2:
                tp2, tp = tp
            else:
                tp2 = None
                tp = tp[0]
            nums = nums.split(',')
            for num in nums:
                if '-' in num:
                    a, b = num.split('-')
                    a = int(a)
                    b = int(b)
                    ranges[i][(a, b)] = (tp2, tp)
                else:
                    a = int(num)
                    ranges[i][(a,)] = (tp2, tp)

    filenames = [f for f in glob(f'logs/{dataset}/**', recursive=True)
                 if os.path.basename(f).isnumeric()]
    rows = []
    for filename in filenames:
        ID = int(os.path.basename(filename))
        with open(filename) as f, open(filename + '_processed', 'w') as g, open(filename + '_metrics', 'w') as m:
            powerMetrics = []
            L = []
            G = []
            for line in f:
                if line.startswith('L'):
                    epochs = line[2:].strip().split()
                    L.extend(epochs)
                elif line.startswith('G'):
                    epochs = line[2:].strip().split()
                    G.extend(epochs)
                else:
                    clock, cpu, lpm, send, listen = map(int, line.strip().split())
                    nonRadioTime = cpu + lpm
                    clock /= TICKS_PER_SECOND
                    cpu *= POWER_CPU/nonRadioTime
                    lpm *= POWER_LPM/nonRadioTime
                    send *= POWER_TRANSMIT/nonRadioTime
                    listen *= POWER_LISTEN/nonRadioTime
                    powerMetrics.append((clock, cpu, lpm, send, listen))

            print('Power Usage', file=g)
            for clock, cpu, lpm, send, listen in powerMetrics:
                print(clock, cpu, lpm, send, listen, file=g)
            print(file=g)
            print('Local Anomalies', file=g)
            print(' '.join(L), file=g)
            print(file=g)
            print('Global Anomalies', file=g)
            print(' '.join(G), file=g)

            trueL = set()
            trueG = set()
            for k in ranges[ID]:
                if len(k) == 2:
                    nums = range(k[0], k[1]+1)
                else:
                    nums = [k[0]]
                if ranges[ID][k][0] != 'G':
                    trueL.update(nums)
                if ranges[ID][k][0] != 'L':
                    trueG.update(nums)

            L = list(map(int, L))
            G = list(map(int, G))

            splits = filename.split(os.path.sep)
            testID = int(splits[-2][4:])
            testType = splits[-3]
            row = [testID, testType, ID]
            if len(L) > 0 and len(trueL) > 0:
                Ltp = len(trueL.intersection(L))
                Lcm = [[Ltp, len(L) - Ltp],
                       [len(trueL) - Ltp, totalEpochs[dataset] + Ltp - len(L) - len(trueL)]]
                Ltpr = Lcm[0][0]/len(trueL)
                Ltnr = Lcm[0][0]/(totalEpochs[dataset] - len(trueL))
                Lprecision = Lcm[0][0]/len(L)
                LBMscore = Ltpr + Ltnr - 1
                print("Local Anomaly Confusion Matrix:", file=m)
                print("{}, {}\n{}, {}".format(Lcm[0][0], Lcm[0][1], Lcm[1][0], Lcm[1][1]), file=m)
                print(file=m)
                print("{:>7s}{:>12s}{:>12s}".format('TPR', 'Precision', 'BM Score'), file=m)
                print("{:7.2f}{:12.2f}{:12.2f}".format(Ltpr*100, Lprecision*100, LBMscore), file=m)
                print(file=m)
                row.extend([Lcm[0][0], Lcm[0][1], Lcm[1][0], Lcm[1][1], Ltpr, Lprecision, LBMscore])
            else:
                row.extend([None]*7)
            if len(G) > 0 and len(trueG) > 0:
                Gtp = len(trueG.intersection(G))
                Gcm = [[Gtp, len(G) - Gtp],
                       [len(trueG) - Gtp, totalEpochs[dataset] + Gtp - len(G) - len(trueG)]]
                Gtpr = Gcm[0][0]/len(trueG)
                Gtnr = Gcm[0][0]/(totalEpochs[dataset] - len(trueL))
                Gprecision = Gcm[0][0]/len(G)
                GBMscore = Gtpr + Gtnr - 1
                print("Global Anomaly Confusion Matrix:", file=m)
                print("{}, {}\n{}, {}".format(Gcm[0][0], Gcm[0][1], Gcm[1][0], Gcm[1][1]), file=m)
                print(file=m)
                print("{:>7s}{:>12s}{:>12s}".format('TPR', 'Precision', 'BM Score'), file=m)
                print("{:7.2f}{:12.2f}{:12.2f}".format(Gtpr*100, Gprecision*100, GBMscore), file=m)
                row.extend([Gcm[0][0], Gcm[0][1], Gcm[1][0], Gcm[1][1], Gtpr, Gprecision, GBMscore])
            else:
                row.extend([None]*7)
            rows.append(row)
    df = pd.DataFrame(rows, columns=('TestID', 'Test', 'NodeID', 'LTP', 'LFP', 'LFN', 'LTN', 'Ltpr', 'Lprec', 'LBM',
                                     'GTP', 'GFP', 'GFN', 'GTN', 'Gtpr', 'Gprec', 'GBM'))
    df = df.astype({'TestID': int})
    df = pd.merge(tests, df, on='TestID')
    df = df.set_index('TestID')
    df = df.sort_values(['Test', 'NodeID'])
    df.to_html(f'logs/{dataset}/results.html')
    df.to_excel(f'logs/{dataset}/results.xlsx', 'Data')
    df_all[dataset] = df
df = pd.concat(df_all, join='inner', names=['Dataset'])
df = df.reset_index()
df.to_excel(f'logs/all.xlsx', 'Data')
