import subprocess
import argparse
import pandas as pd

parser = argparse.ArgumentParser()
parser.add_argument('-t', dest='testIDs', required=True, nargs='+')
parser.add_argument('-d', dest='datasets', required=True, nargs='+')
args = parser.parse_args()

tests = pd.read_csv('logs/tests.txt', sep=' ', index_col='TestID')
for testID in args.testIDs:
    testID = int(testID)
    test = tests.loc[testID, 'Test']
    algorithm = tests.loc[testID, 'Algorithm']

    if algorithm == 'Original':
        subprocess.run("cp -v anomaly-original.c anomaly.c", cwd='/home/aaron/src/anomaly', shell=True)
    elif algorithm == 'Adaptive':
        subprocess.run("cp -v anomaly-adaptive.c anomaly.c", cwd='/home/aaron/src/anomaly', shell=True)
    else:
        subprocess.run("cp -v anomaly-periodic.c anomaly.c", cwd='/home/aaron/src/anomaly', shell=True)

    subprocess.run(f"sed -i -E '/#define NU/ s/[0-9\\.]+/{tests.loc[testID, 'nu']}/' include/common.h", shell=True)
    subprocess.run(f"sed -i -E '/#define NUM_READINGS/ s/[0-9\\.]+/{tests.loc[testID, 'n']}/' include/common.h", shell=True)
    if tests.loc[testID, 'M'] > 0:
        subprocess.run(f"sed -i -E '/#define M/ s/[0-9\\.]+/{tests.loc[testID, 'M']}/' include/common.h", shell=True)
    subprocess.run(f"sed -i -E '/#define SIG/ s/[0-9\\.]+/{tests.loc[testID, 'sigma']}/' include/common.h", shell=True)

    for dataset in args.datasets:
        subprocess.run(f"mkdir -v -p $HOME/src/anomaly/logs/{dataset}/{test}/Test{testID}", shell=True)
        subprocess.run(['ant', 'run_nogui', f'-Dargs=/home/aaron/src/anomaly/{dataset}.csc'], cwd='/home/aaron/src/contiki/tools/cooja')
        subprocess.run(f"mv -v $HOME/src/anomaly/logs/{dataset}/[0-9]* $HOME/src/anomaly/logs/{dataset}/{test}/Test{testID}", shell=True)
