# -*- coding: UTF-8 -*-
# libraries and data
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import glob
import os
import sys

def battery(main_path, teste, uavs_id, time_ini, time_end):
    plt.clf()
    uavs = []
    data = {}
    filtred = []
    if teste:
        print uavs_id
    for name_file in glob.glob(main_path+'uav_battery_all_*.txt'):
        base=os.path.basename(name_file)
        uav_id = os.path.splitext(base)[0].split("uav_battery_all_")[-1]
        if uav_id in uavs_id:
            uav_id = "UAV "+str(uav_id)
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
                if time_ini < point[0] < time_end: # just between the snapshot
                    filtred.append(point[1])
                data[uav_id][0].append(point[0])
                data[uav_id][1].append(point[1])
            file.close()

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
        plt.xlim(time_ini-2, time_end+2)
        plt.ylim(min(filtred), max(filtred))

        # # Not ticks everywhere
        # if num in range(7) :
        #     ax.tick_params(labelbottom='off')
        # if num not in [1,4,7] :

        if num != number_plots:
            plt.tick_params(labelbottom=False)

        # Add title
        plt.title(str(uav), loc='left', fontsize=12, fontweight=0, color=palette(num) )

    # general title
    plt.suptitle("Consumo de bateria por flutuar dos UAVs entre os tempos ["+str(time_ini)+"s, "+str(time_end)+"s]", fontsize=13, fontweight=0, color='black', style='italic')

    plt.xlabel('Tempo (s)')
    plt.ylabel('Bateria (%)')

    plt.savefig(main_path+'battery_all_'+str(time_end)+'.svg')
    plt.savefig(main_path+'battery_all_'+str(time_end)+'.eps')
    plt.savefig(main_path+'battery_all_'+str(time_end)+'.png')
