
# -*- coding: UTF-8 -*-
# libraries and data
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import glob
import os
import sys

def battery(main_path, teste, uav_bat):
    uavs = []
    data = {}
    if teste:
        print uav_bat
    for name_file in glob.glob(main_path+'uav_battery_*.txt'):
        base=os.path.basename(name_file)
        if str(os.path.splitext(base)[0]) in uav_bat:
            uav_id = "UAV "+os.path.splitext(base)[0].split("uav_battery_")[-1]
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

    # if teste:
    #     print ('Data: ' + str(data))
    #     print ('UAVs: ' + str(uavs))


    # Initialize the figure
    plt.style.use('seaborn-darkgrid')

    # create a color palette
    palette = plt.get_cmap('Set1')

    number_plots = len(uavs)

    # multiple line plot
    num=0
    for uav in uavs:
        num+=1

        # Find the right spot on the plot
        plt.subplot(number_plots, 1, num) #(3,3, num)

        # plot every groups, but discreet
        for key, value in data.iteritems():
            plt.plot(value[0], value[1], marker='', color='grey', linewidth=0.6, alpha=0.3)

        # Plot the lineplot
        plt.plot(data[uav][0], data[uav][1], marker='', color=palette(num), linewidth=2.4, alpha=0.9, label=uav)

        # Same limits for everybody!
        # ax.xlim(0,10)
        plt.ylim(0,1.0)

        # # Not ticks everywhere
        # if num in range(7) :
        #     ax.tick_params(labelbottom='off')
        # if num not in [1,4,7] :

        if num != number_plots:
            plt.tick_params(labelbottom=False)

        # Add title
        plt.title(str(uav), loc='left', fontsize=12, fontweight=0, color=palette(num) )

    # general title
    plt.suptitle("Consumo de bateria dos UAVs ao longo do tempo", fontsize=13, fontweight=0, color='black', style='italic')

    plt.xlabel('Tempo (s)')
    plt.ylabel('Bateria (%)')

    plt.savefig(main_path+'battery_graphic.svg')
    plt.savefig(main_path+'battery_graphic.eps')
    plt.savefig(main_path+'battery_graphic.png')
