
# -*- coding: UTF-8 -*-
# libraries and data
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import seaborn as sns
import glob
import os
import sys

def bij (custo, time, main_path, teste):
    uav_bat = {}
    uav_mov = {}
    data_bij = {}
    try:
        file = open(main_path+custo+'/'+'etapa/'+time+"/bij.txt", 'r')
    except IOError:
        return
    line = file.readline().strip()
    uavs = [x for x in line.split(',')]

    print "Uavs:"+str(uavs)

    for i in uavs:
        uav_mov[str("uav_moving_"+str(int(i)))] = 1

    line = file.readline().strip()
    locs = [x for x in line.split(',')]

    print "Locs:"+str(locs)

    cont = 0
    for line in file:
        data_bij[uavs[cont]] = [float(x) for x in line.split(',')] # read row from file
        cont = cont + 1
    file.close()

    plt.close()
    cmap = sns.cubehelix_palette(50, hue=0.05, rot=0, light=0.9, dark=0, as_cmap=True)

    # Create a dataset
    df_bij = pd.DataFrame(data_bij, index=locs)

    # Default heatmap: just a visualization of this square matrix
    sns.heatmap(df_bij, cmap=cmap, vmin=0, vmax=1)

    plt.xlabel("UAV")
    plt.ylabel(u"Location")

    # general title
    # plt.title("Bij", fontsize=13, fontweight=0, color='black', style='italic')

    plt.savefig(main_path+custo+'/'+'etapa/'+time+'/bij.svg')
    plt.savefig(main_path+custo+'/'+'etapa/'+time+'/bij.png')
    plt.savefig(main_path+custo+'/'+'etapa/'+time+'/bij.eps')

    plt.close()
    cmap = sns.cubehelix_palette(50, hue=0.05, rot=0, light=0.9, dark=0, as_cmap=True)
    # Default heatmap: just a visualization of this square matrix
    sns.heatmap(df_bij, cmap=cmap)

    plt.xlabel("UAV")
    plt.ylabel(u"Location")

    # general title
    # plt.title("Bij", fontsize=13, fontweight=0, color='black', style='italic')

    plt.savefig(main_path+custo+'/'+'etapa/'+time+'/bij_.svg')
    plt.savefig(main_path+custo+'/'+'etapa/'+time+'/bij_.png')
    plt.savefig(main_path+custo+'/'+'etapa/'+time+'/bij_.eps')
    plt.close()

    return (uavs, uav_mov)
