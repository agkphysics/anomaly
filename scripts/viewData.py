import pandas as pd
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
# import numpy as np
import sys
from sklearn.decomposition import PCA
from sklearn.preprocessing import StandardScaler

colours = {'NoiseW': 'gray', 'NoiseO': 'green', 'ClusterO': 'red', 'ClusterW': 'blue'}
ranges = {1: {}}

if len(sys.argv) < 2:
    print('Please specify which dataset to view.', file=sys.stderr)
    sys.exit(1)

with open(f'data/truth/{sys.argv[1]}.txt') as f:
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
            tp = tp[1]
        else:
            tp = tp[0]
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

if sys.argv[1] == 'HIWS':
    df = pd.read_csv('data/HIWS', sep=' ', names=('Epoch', 'WSPD', 'WDIR', 'AIRT', 'ATMP', 'RELH', 'RAIN'), index_col='Epoch')
    c = ['black']*len(df.index)
    for nums in ranges[1]:
        if len(nums) == 1:
            c[nums[0]] = colours[ranges[1][nums]]
        else:
            for i in range(nums[0], nums[1]+1):
                c[i] = colours[ranges[1][nums]]

    pca = PCA(3, whiten=True)
    std = StandardScaler().fit_transform(df)
    x = pca.fit_transform(df)
    fig = plt.figure()
    ax = Axes3D(fig)
    plt.title("Sensor Node")
    sc = ax.scatter(*x.T, c=c)
    # plt.scatter(*x.T, c=df.index)

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
elif sys.argv[1] == 'HITEMP':
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
elif sys.argv[1] == 'StBernard':
    for ID in range(1, 6):
        df = pd.read_csv(f'data/StBernard_{ID}',
                         sep=' ',
                         names=('Epoch', 'Ambient', 'Surface', 'Radiation', 'Humidity',
                                'SoilMoisture', 'Watermark', 'Rain', 'WindSpeed', 'WindDir'),
                         index_col='Epoch')
        pca = PCA(3, whiten=True)
        std = StandardScaler().fit_transform(df)
        x = pca.fit_transform(std)
        fig = plt.figure()
        ax = Axes3D(fig)
        ax.set_title(f"Sensor Node {ID}")
        ax.set_xlabel('PC1')
        ax.set_ylabel('PC2')
        ax.set_zlabel('PC3')
        ax.scatter(*x.T, c=df.index)
        # ax.plot(*x.T)
        for i in range(df.shape[0]):
            ax.text(x[i, 0], x[i, 1], x[i, 2], i)
        figManager = plt.get_current_fig_manager()
        figManager.window.showMaximized()
        plt.show()
