import pandas as pd
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
# import numpy as np
import sys
from sklearn.decomposition import PCA
from sklearn.preprocessing import StandardScaler
import argparse
from glob import glob
import os.path

colours = {'Noise': 'green', 'LocalCluster': 'blue', 'Environment': 'red'}
ranges = {}

parser = argparse.ArgumentParser(description="View data with various formatting")
parser.add_argument('dataset', help="the dataset to view")
parser.add_argument('--std', action='store_true', help="standardise the data")
parser.add_argument('--text', action='store_true', help="show epoch labels")
subparsers = parser.add_subparsers(dest='type', required=True, title="type of data")
subparsers.add_parser('truth')
subparsers.add_parser('epoch')
testParser = subparsers.add_parser('test')
testParser.add_argument('test', metavar='N', type=int, help="the test to view")

args = parser.parse_args()
dataset = args.dataset
standardise = args.std

if args.type == 'truth':
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
elif args.type == 'test':
    files = [f for f in glob(f'logs/{dataset}/**/Test{args.test}/*')
             if os.path.basename(f).isnumeric()]
    for file in files:
        ID = int(os.path.basename(file))
        ranges[ID] = {}
        with open(f'{file}_processed') as f:
            for line in f:
                if line.startswith('Anomalies'):
                    break
            anomalies = list(map(int, next(f).strip().split()))
            for a in anomalies:
                ranges[ID][(a,)] = 'Noise'

pca = PCA(3)
std = StandardScaler()

if dataset == 'HIWS':
    df = pd.read_csv('data/HIWS',
                     sep=' ',
                     names=('Epoch', 'WSPD', 'WDIR', 'AIRT', 'ATMP', 'RELH', 'RAIN'),
                     index_col='Epoch')
    if args.std:
        x = pca.fit_transform(std.fit_transform(df))
    else:
        x = pca.fit_transform(df)

    fig = plt.figure()
    ax = Axes3D(fig)
    plt.title("Sensor Node")
    ax.set_xlabel('PC1')
    ax.set_ylabel('PC2')
    ax.set_zlabel('PC3')

    if args.type in ['truth', 'test']:
        c = ['black']*len(df.index)
        for nums in ranges[1]:
            if len(nums) == 1:
                c[nums[0]] = colours[ranges[1][nums]]
            else:
                for i in range(nums[0], nums[1]+1):
                    c[i] = colours[ranges[1][nums]]
        sc = ax.scatter(*x.T, c=c, depthshade=False)
    else:
        sc = ax.scatter(*x.T, c=df.index, depthshade=False)
        cbar = fig.colorbar(sc, ax=ax, fraction=0.1, shrink=0.8)
        cbar.set_label('Epoch')
    if args.text:
        for i in range(df.shape[0]):
            ax.text(x[i, 0], x[i, 1], x[i, 2], i)
    plt.show()
    sys.exit()

    for var in 'WSPD', 'WDIR', 'AIRT', 'ATMP', 'RELH', 'RAIN':
        plt.figure()
        plt.title(var)
        plt.plot(df.index, df.loc[:, var])
        for nums in ranges[1]:
            xmin, xmax, ymin, ymax = plt.axis()
            if len(nums) == 1:
                rect = plt.Rectangle([nums[0], ymin], 1, ymax - ymin, color=colours[ranges[1][nums]], alpha=0.3)
            else:
                rect = plt.Rectangle([nums[0], ymin], nums[1] - nums[0], ymax - ymin, color=colours[ranges[1][nums]], alpha=0.3)
            plt.gca().add_patch(rect)
        plt.xlabel('Epoch')
        plt.ylabel(var)
        plt.draw()
    plt.show()
elif dataset == 'HITEMP':
    fig = plt.figure()
    ax = plt.axes()
    ax.set_title('Combined water temperature data')
    ax.set_xlabel('Epoch')
    ax.set_ylabel('TEMP')
    for ID in range(1, 11):
        df = pd.read_csv(f'data/HITEMP_{ID}', sep=' ', names=('Epoch', 'TEMP'), index_col='Epoch')
        ax.plot(df.index, df.loc[:, 'TEMP'])
    for ID in range(1, 11):
        for nums in ranges[ID]:
            xmin, xmax, ymin, ymax = ax.axis()
            if len(nums) == 1:
                rect = plt.Rectangle([nums[0], ymin], 1, ymax - ymin, color=colours[ranges[ID][nums]], alpha=0.1)
            else:
                rect = plt.Rectangle([nums[0], ymin], nums[1] - nums[0], ymax - ymin, color=colours[ranges[ID][nums]], alpha=0.1)
            ax.add_patch(rect)
    plt.show()

    for ID in range(1, 11):
        plt.figure()
        plt.xlabel('Epoch')
        plt.ylabel('TEMP')
        plt.title(f'Sensor node {ID}')
        df = pd.read_csv(f'data/HITEMP_{ID}', sep=' ', names=('Epoch', 'TEMP'), index_col='Epoch')
        plt.plot(df.index, df.loc[:, 'TEMP'])
        for nums in ranges[ID]:
            xmin, xmax, ymin, ymax = ax.axis()
            if len(nums) == 1:
                rect = plt.Rectangle([nums[0], ymin], 1, ymax - ymin, color=colours[ranges[ID][nums]], alpha=0.1)
            else:
                rect = plt.Rectangle([nums[0], ymin], nums[1] - nums[0], ymax - ymin, color=colours[ranges[ID][nums]], alpha=0.1)
            plt.gca().add_patch(rect)
        plt.show()
elif dataset == 'StBernard':
    dfAll = []
    for ID in [1, 2, 3, 4, 5]:
        df = pd.read_csv(f'data/StBernard_{ID}',
                         sep=' ',
                         names=('Epoch', 'Ambient', 'Surface', 'Radiation', 'Humidity',
                                'SoilMoisture', 'Watermark', 'Rain', 'WindSpeed', 'WindDir'),
                         index_col='Epoch')
        dfAll.append(df)
    dfAll = pd.concat(dfAll, keys=[1, 2, 3, 4, 5])
    std = std.fit(dfAll)
    if args.std:
        pca = pca.fit(std.transform(dfAll))
    else:
        pca = pca.fit(dfAll)
    fig = plt.figure()
    ax = Axes3D(fig)
    ax.set_title(f"All Nodes")
    ax.set_xlabel('PC1')
    ax.set_ylabel('PC2')
    ax.set_zlabel('PC3')
    for ID in [1, 2, 3, 4, 5]:
        df = dfAll.loc[ID, :]
        if args.std:
            x = pca.transform(std.transform(df))
        else:
            x = pca.transform(df)
        ax.scatter(*x.T, s=1)
    plt.show()

    for ID in [1, 2, 3, 4, 5]:
        df = dfAll.loc[ID, :]
        if args.std:
            x = pca.transform(std.transform(df))
        else:
            x = pca.transform(df)

        fig = plt.figure()
        ax = Axes3D(fig)
        ax.set_title(f"Sensor Node {ID}")
        ax.set_xlabel('PC1')
        ax.set_ylabel('PC2')
        ax.set_zlabel('PC3')

        if args.type in ['truth', 'test']:
            c = ['black']*len(df.index)
            for nums in ranges[ID]:
                if len(nums) == 1:
                    c[nums[0]] = colours[ranges[ID][nums]]
                else:
                    for i in range(nums[0], nums[1]+1):
                        c[i] = colours[ranges[ID][nums]]
            ax.scatter(*x.T, c=c)
        else:
            ax.scatter(*x.T, c=df.index)
        if args.text:
            for i in range(df.shape[0]):
                ax.text(x[i, 0], x[i, 1], x[i, 2], i)

        # figManager = plt.get_current_fig_manager()
        # figManager.window.showMaximized()
        plt.show()
elif dataset == 'Banana2' or dataset.startswith('Noise'):
    for ID in [1, 2, 3]:
        df = pd.read_csv(f'data/{dataset}_{ID}',
                         sep=' ',
                         names=('Epoch', 'V1', 'V2'),
                         index_col='Epoch')
        fig = plt.figure()
        ax = plt.gca()
        ax.set_title(f"Sensor Node {ID}")
        ax.set_xlabel('V1')
        ax.set_ylabel('V2')
        if args.type in ['truth', 'test']:
            c = ['black']*len(df.index)
            for nums in ranges[ID]:
                if len(nums) == 1:
                    c[nums[0]] = colours[ranges[ID][nums]]
                else:
                    for i in range(nums[0], nums[1]+1):
                        c[i] = colours[ranges[ID][nums]]
            # for i in range(0, df.shape[0], 30):
            #     plt.figure()
            #     plt.scatter(df.V1.iloc[i:i+30], df.V2.iloc[i:i+30], c=c[i:i+30])
            #     plt.show()
            ax.scatter(df.V1, df.V2, c=c)
        else:
            ax.scatter(df.V1, df.V2, c=df.index)
        if args.text:
            for i in range(df.shape[0]):
                ax.text(df.loc[i, 'V1'], df.loc[i, 'V2'], i)
        plt.show()
