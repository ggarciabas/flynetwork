
# -*- coding: UTF-8 -*-
# libraries and data
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import seaborn as sns
import glob
import os
import sys

def mij(time, main_path, teste):
    data_mij = {}
    file = open(main_path+'etapa/'+time+'/f_mij.txt', 'r')
    line = file.readline().strip()
    graphic_temp = [x for x in line.split(',')]
    line = file.readline().strip()
    uavs = [x for x in line.split(',')]
    print uavs
    line = file.readline().strip()
    locs = [x for x in line.split(',')]
    cont = 0
    for line in file:
        print line
        val = line.split(',')
        print val
        data_mij[uavs[cont]] = [float(x) for x in val] # read row from file
        cont = cont + 1
    file.close()

    plt.clf()
    cmap = sns.cubehelix_palette(50, hue=0.05, rot=0, light=0.9, dark=0, as_cmap=True)

    # Create a dataset
    df_mij = pd.DataFrame(data_mij, index=locs)

    # Default heatmap: just a visualization of this square matrix
    pic = sns.heatmap(df_mij, cmap=cmap, vmin=0, vmax=1)

    # plt.title("Mij - Temp. "+graphic_temp[0])
    plt.xlabel("Location")
    plt.ylabel("UAV")

    plt.savefig(main_path+'etapa/'+time+'/f_mij.svg')
    plt.savefig(main_path+'etapa/'+time+'/f_mij.png')
    plt.savefig(main_path+'etapa/'+time+'/f_mij.eps')

    plt.clf()
    cmap = sns.cubehelix_palette(50, hue=0.05, rot=0, light=0.9, dark=0, as_cmap=True)

    # Default heatmap: just a visualization of this square matrix
    pic = sns.heatmap(df_mij, cmap=cmap)

    # plt.title("Mij - Temp. "+graphic_temp[0])
    plt.xlabel("Location")
    plt.ylabel("UAV")

    plt.savefig(main_path+'etapa/'+time+'/f_mij_.svg')
    plt.savefig(main_path+'etapa/'+time+'/f_mij_.png')
    plt.savefig(main_path+'etapa/'+time+'/f_mij_.eps')
