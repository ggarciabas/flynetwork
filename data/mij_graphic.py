
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
    # cada arquivo vira um gráfico, para ver evolução de acordo com a temperatura
    list_file=[]
    for file_path in glob.glob(main_path+time+'/mij_*.txt'):
        file_name = os.path.basename(file_path)
        if teste:
            print (file_name)
        list_file.append(file_name)

    if len(list_file) == 0:
        return

    list_file = np.array(list_file)
    list_file.sort()

    if teste:
        print (list_file)

    selected_file = []
    selected_file.append(list_file[1]) # primeiro
    selected_file.append(list_file[(len(list_file)/6)*3]) # meio
    selected_file.append(list_file[-1]) # ultimo

    for lfile in selected_file:
        data_mij = {}
        file = open(main_path+time+'/'+lfile, 'r')
        line = file.readline().strip()
        graphic_temp = [x for x in line.split(',')]
        line = file.readline().strip()
        uavs = [x for x in line.split(',')]
        line = file.readline().strip()
        locs = [x for x in line.split(',')]
        cont = 0
        for line in file:
            data_mij[uavs[cont]] = [float(x) for x in line.split(',')] # read row from file
            cont = cont + 1
        file.close()

        plt.clf()
        cmap = sns.cubehelix_palette(50, hue=0.05, rot=0, light=0.9, dark=0, as_cmap=True)

        # Create a dataset
        df_mij = pd.DataFrame(data_mij, index=locs)

        # Default heatmap: just a visualization of this square matrix
        pic = sns.heatmap(df_mij, cmap=cmap, vmin=0, vmax=1)

        plt.title("Mij - Temp. "+graphic_temp[0])
        plt.ylabel("Localizacao")
        plt.xlabel("UAV")

        plt.savefig(main_path+time+'/'+lfile+'.svg')
        plt.savefig(main_path+time+'/'+lfile+'.png')
        plt.savefig(main_path+time+'/'+lfile+'.eps')

        plt.clf()
        cmap = sns.cubehelix_palette(50, hue=0.05, rot=0, light=0.9, dark=0, as_cmap=True)

        # Default heatmap: just a visualization of this square matrix
        pic = sns.heatmap(df_mij, cmap=cmap)

        plt.title("Mij - Temp. "+graphic_temp[0])
        plt.ylabel("Localizacao")
        plt.xlabel("UAV")

        plt.savefig(main_path+time+'/'+lfile+'_.svg')
        plt.savefig(main_path+time+'/'+lfile+'_.png')
        plt.savefig(main_path+time+'/'+lfile+'_.eps')
