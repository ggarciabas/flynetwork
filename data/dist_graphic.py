
# -*- coding: UTF-8 -*-
# libraries and data
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import seaborn as sns
import glob
import os
import sys

def dist (time, main_path, teste):
    try:
        file = open(main_path+time+"/dist.txt", 'r')
    except IOError:
        return
    data_dist = {}
    line = file.readline().strip()
    uavs = [x for x in line.split(',')]
    line = file.readline().strip()
    locs = [x for x in line.split(',')]
    cont = 0
    for line in file:
        data_dist[uavs[cont]] = [float(x) for x in line.split(',')] # read row from file
        cont = cont + 1
    file.close()

    plt.clf()
    cmap = sns.cubehelix_palette(50, hue=0.05, rot=0, light=0.9, dark=0, as_cmap=True)

    # Create a dataset
    df_dist = pd.DataFrame(data_dist, index=locs)

    # Default heatmap: just a visualization of this square matrix
    sns.heatmap(df_dist, cmap=cmap, vmin=0, vmax=1)

    plt.ylabel("Localizacao")
    plt.xlabel("UAV")

    # general title
    plt.title("Distance (battery)", fontsize=13, fontweight=0, color='black', style='italic')

    plt.savefig(main_path+time+'/dist.svg')
    plt.savefig(main_path+time+'/dist.png')
    plt.savefig(main_path+time+'/dist.eps')

    plt.clf()
    cmap = sns.cubehelix_palette(50, hue=0.05, rot=0, light=0.9, dark=0, as_cmap=True)
    # Default heatmap: just a visualization of this square matrix
    sns.heatmap(df_dist, cmap=cmap)

    plt.ylabel("Localizacao")
    plt.xlabel("UAV")

    # general title
    plt.title("Distance (battery)", fontsize=13, fontweight=0, color='black', style='italic')

    plt.savefig(main_path+time+'/dist_.svg')
    plt.savefig(main_path+time+'/dist_.png')
    plt.savefig(main_path+time+'/dist_.eps')
