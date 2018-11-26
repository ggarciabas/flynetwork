
# -*- coding: UTF-8 -*-
# libraries and data
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import glob
import os
import sys

def battery(main_path, teste, uav_bat):
    plt.clf()
    print 'Battery Graphic'
    uavs = []
    data = {}
    if teste:
        print uav_bat
    for name_file in glob.glob(main_path+'uav_battery_graphic_*.txt'):
        base=os.path.basename(name_file)
        # if str(os.path.splitext(base)[0]) in uav_bat:
        uav_id = "UAV "+os.path.splitext(base)[0].split("uav_battery_graphic_")[-1]
        if teste:
            print (os.path.splitext(base)[0])
            print (uav_id)
        uavs.append(uav_id)
        if teste:
            print uavs
        data[uav_id] = []
        data[uav_id].append([]) # x
        data[uav_id].append([]) # y
        file = open(name_file, 'r')
        for line in file:
            # if teste:
            #     print ('Line: ' + str(line))
            point = [float(x) for x in line.split(',')]
            data[uav_id][0].append(point[0])
            data[uav_id][1].append(point[1])
        file.close()

    # Initialize the figure
    plt.style.use('seaborn-darkgrid')

    fig = plt.figure()
    ax = fig.add_subplot(111)
    for uav in uavs:
        # Plot the lineplot
        plt.plot(data[uav][0], data[uav][1], marker='', linewidth=2.4, alpha=0.9, label=uav)

    # general title
    plt.suptitle("Consumo de bateria dos UAVs ao longo do tempo", fontsize=13, fontweight=0, color='black', style='italic')
    lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.10), fancybox=True, shadow=True, ncol=8, fontsize=7)
    plt.xlabel('Tempo (s)')
    plt.ylabel('Bateria (%)')

    plt.savefig(main_path+'battery_graphic.svg')
    plt.savefig(main_path+'battery_graphic.eps')
    plt.savefig(main_path+'battery_graphic.png')
