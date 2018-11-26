
# -*- coding: UTF-8 -*-
# libraries and data
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import seaborn as sns
import glob
import os
import sys

def cli (time, main_path, teste):
    try:
        file = open(main_path+time+"/cli.txt", 'r')
    except IOError:
        return
    data_cli = {}
    line = file.readline().strip()
    uavs = [x for x in line.split(',')]
    line = file.readline().strip()
    locs = [x for x in line.split(',')]
    cont = 0
    for line in file:
        data_cli[uavs[cont]] = [float(x) for x in line.split(',')] # read row from file
        cont = cont + 1
    file.close()

    plt.clf()
    cmap = sns.cubehelix_palette(50, hue=0.05, rot=0, light=0.9, dark=0, as_cmap=True)

    # Create a dataset
    df_cli = pd.DataFrame(data_cli, index=locs)

    # Default heatmap: just a visualization of this square matrix
    sns.heatmap(df_cli, cmap=cmap, vmin=0, vmax=1)

    plt.ylabel("Localizacao")
    plt.xlabel("UAV")

    # general title
    plt.title("Client (%)", fontsize=13, fontweight=0, color='black', style='italic')

    plt.savefig(main_path+time+'/cli.svg')
    plt.savefig(main_path+time+'/cli.png')
    plt.savefig(main_path+time+'/cli.eps')

    plt.clf()
    cmap = sns.cubehelix_palette(50, hue=0.05, rot=0, light=0.9, dark=0, as_cmap=True)
    # Default heatmap: just a visualization of this square matrix
    sns.heatmap(df_cli, cmap=cmap)

    plt.ylabel("Localizacao")
    plt.xlabel("UAV")

    # general title
    plt.title("Client (%)", fontsize=13, fontweight=0, color='black', style='italic')

    plt.savefig(main_path+time+'/cli_.svg')
    plt.savefig(main_path+time+'/cli_.png')
    plt.savefig(main_path+time+'/cli_.eps')
