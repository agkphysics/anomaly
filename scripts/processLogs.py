# processLogs.py - Script to extract algorithm performance and power usage
#     from log files
#
# Author: Aaron Keesing

import os.path
from glob import glob

TICKS_PER_SECOND = 4096
VOLTAGE = 3
POWER_CPU = 1.800*VOLTAGE      # mW
POWER_LPM = 0.0545*VOLTAGE     # mW
POWER_TRANSMIT = 17.7*VOLTAGE  # mW
POWER_LISTEN = 20.0*VOLTAGE    # mW

filenames = [f for f in glob('logs/**', recursive=True) if os.path.isfile(f) and not f.endswith('processed')]
for filename in filenames:
    with open(filename) as f, open(filename + '_processed', 'w') as g:
        powerMetrics = []
        localAnomalies = []
        globalAnomalies = []
        for line in f:
            if line.startswith('L'):
                epochs = line[2:].strip().split()
                localAnomalies.extend(epochs)
            elif line.startswith('G'):
                epochs = line[2:].strip().split()
                globalAnomalies.extend(epochs)
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
        print(' '.join(localAnomalies), file=g)
        print(file=g)
        print('Global Anomalies', file=g)
        print(' '.join(globalAnomalies), file=g)
