import pandas as pd
import matplotlib.pyplot as plt
import sys

colours = {'Noise': 'gray', 'Cluster': 'red', 'Shift': 'blue'}
ranges = {}

if len(sys.argv) < 2:
    print('Please specify which dataset to view.', file=sys.stderr)
    sys.exit(1)

if sys.argv[1] == 'HIWS':
    df = pd.read_csv('data/HIWS', sep=' ', names=('Epoch', 'WSPD', 'WDIR', 'AIRT', 'ATMP', 'RELH', 'RAIN'))

    with open('data/truth/HIWS_truth.txt') as f:
        for line in f:
            nums, tp = line.split()
            if '-' in nums:
                a, b = nums.split('-')
                a = int(a)
                b = int(b)
                ranges[(a, b)] = tp
            else:
                a = int(nums)
                ranges[(a,)] = tp

    for var in 'WSPD', 'WDIR', 'AIRT', 'ATMP', 'RELH', 'RAIN':
        plt.figure()
        plt.title(var)
        plt.plot(df.Epoch, df.loc[:, var])
        for nums in ranges:
            xmin, xmax, ymin, ymax = plt.axis()
            if len(nums) == 1:
                rect = plt.Rectangle([nums[0], ymin], 1, ymax - ymin, color=colours[ranges[nums]], alpha=0.3)
            else:
                rect = plt.Rectangle([nums[0], ymin], nums[1] - nums[0], ymax - ymin, color=colours[ranges[nums]], alpha=0.3)
            plt.gca().add_patch(rect)
        plt.xlabel('Epoch')
        plt.ylabel(var)
        plt.draw()
    plt.show()
elif sys.argv[1] == 'HITEMP':
    for ID in range(1, 11):
        df = pd.read_csv(f'data/HITEMP_{ID}', sep=' ', names=('Epoch', 'TEMP'))
        plt.figure()
        plt.title(ID)
        plt.plot(df.Epoch, df.loc[:, 'TEMP'])
        for nums in ranges:
            xmin, xmax, ymin, ymax = plt.axis()
            if len(nums) == 1:
                rect = plt.Rectangle([nums[0], ymin], 1, ymax - ymin, color=colours[ranges[nums]], alpha=0.3)
            else:
                rect = plt.Rectangle([nums[0], ymin], nums[1] - nums[0], ymax - ymin, color=colours[ranges[nums]], alpha=0.3)
            plt.gca().add_patch(rect)
        plt.xlabel('Epoch')
        plt.ylabel('TEMP')
        plt.draw()
    plt.show()
