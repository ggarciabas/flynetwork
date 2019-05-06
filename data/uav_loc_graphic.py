
# -*- coding: UTF-8 -*-
# libraries and data
import sys
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from matplotlib import colors as mcolors

colors = dict(mcolors.BASE_COLORS, **mcolors.CSS4_COLORS)


def uav_loc (time, main_path, teste, raio_cli, raio_uav):
    try:
        f_cen = open(main_path+'etapa/'+time+'/uav_loc.txt','r')
    except IOError:
        return
    # read LIMITS
    line = f_cen.readline().strip()
    lim = [float(x) for x in line.split(',')]
    # read CENTRAL
    line = f_cen.readline().strip()
    central = [float(x) for x in line.split(',')]
    # read UAV
    line = f_cen.readline().strip()
    uav = [float(x) for x in line.split(',')] # read [x y x y x y] sequence of UAVs
    if teste:
        print (uav)
    # read LOC
    line = f_cen.readline().strip()
    if teste:
        print (line)
    loc = [float(x) for x in line.split(',')] # read [x y x y x y] sequence of Locations
    if teste:
        print (loc)
    #read BIJ
    # line = f_cen.readline().strip()
    # value = [float(x) for x in line.split(',')] # read bij for each x y
    # if teste:
    #     print (value)
    f_cen.close()
    # point_value = range(0,len(value)*2)

    f_cen = open(main_path+'etapa/'+time+'/client.txt','r')
    # read CLIENTS
    line = f_cen.readline().strip()
    cli = [x for x in line.split(',')] # position that server knows
    line = f_cen.readline().strip()
    cli_ = [x for x in line.split(',')] # position real
    f_cen.close()

    fig = plt.figure()
    ax = fig.add_subplot(111)

    # https://matplotlib.org/api/markers_api.html points
    first = True
    app_color = {"VOICE":"darksalmon", "VIDEO":"blueviolet", "WWW":"skyblue", "NOTHING":"yellow", "NONE":"black"}
    apps = {}
    for i in range(0, len(cli_), 4): # x y login app
        if cli_[i+2][0] == "f":
            if first:
                plt.plot(float(cli_[i]),float(cli_[i+1]), 'ks', markersize=7.0, label="fixed")
                first = False
            else:
                plt.plot(float(cli_[i]),float(cli_[i+1]), 'ks', markersize=7.0)
        else:
            if cli_[i+3] in apps: # first
                plt.plot(float(cli_[i]),float(cli_[i+1]), color=colors[app_color[cli_[i+3]]], marker="v", markersize=7.0)
            else:
                apps[cli_[i+3]] = 1
                plt.plot(float(cli_[i]),float(cli_[i+1]), color=colors[app_color[cli_[i+3]]], marker="v", markersize=7.0, label=cli_[i+3])

    firstF = True
    firstM = True
    for i in range(0, len(cli), 3):
        if cli[i+2][0] == "f":
            if firstF:
                plt.plot(float(cli[i]),float(cli[i+1]), 'bs', markersize=7.0, label="fixed")
                firstF = False
            else:
                plt.plot(float(cli[i]),float(cli[i+1]), 'bs', markersize=7.0)
        else:
            if firstM:
                plt.plot(float(cli[i]),float(cli[i+1]), 'k1', markersize=7.0, label="movel")
                firstM = False
            else:
                plt.plot(float(cli[i]),float(cli[i+1]), 'k1', markersize=7.0)

    # cont = 0
    first = True
    for i in range(0, len(uav), 3):
        x = [uav[i+1],loc[i+1]]
        y = [uav[i+2],loc[i+2]]
        if first:
            plt.plot(x,y,'cx-', markersize=7.0, label="localizacao")
            first = False
        else:
            plt.plot(x,y,'cx-', markersize=7.0)
        ax.annotate(str(int(uav[i])), xy=(uav[i+1],uav[i+2]+10))
        ax.annotate(str(int(loc[i])), xy=(loc[i+1],loc[i+2]-10))
        # ax.add_patch(
        #     patches.Circle((loc[i+1],loc[i+2]), radius=raio_cli, color='r', fill=False, linestyle='dashed')
        # )
        # ax.add_patch(
        #     patches.Circle((loc[i+1],loc[i+2]), radius=raio_uav, color='b', fill=False, linestyle='dotted')
        # )
        # point_value[cont] = float(uav[i+1]+loc[i+1])/2.0 # x
        # point_value[cont+1] = float(uav[i+2]+loc[i+2])/2.0 # y
        # cont = cont+1

    first = True
    for i in range(0, len(uav), 3):
        if teste:
            print (str(uav[i])+' '+str(uav[i+1])+' '+str(uav[i+2]))
        if first:
            plt.plot(uav[i+1], uav[i+2],'rD', markersize=5.0, label="uav")
            first = False
        else:
            plt.plot(uav[i+1], uav[i+2], 'rD', markersize=5.0)

        # ax.add_patch(
        #     patches.Circle((uav[i+1], uav[i+2]), radius=raio_cli, color='r', fill=False, linestyle='dashed')
        # )
        # ax.add_patch(
        #     patches.Circle((uav[i+1], uav[i+2]), radius=raio_uav, color='b', fill=False, linestyle='dotted')
        # )


    plt.plot(central[0],central[1],'g*', markersize=7.0, label="central")
    # ax.add_patch(
    #     patches.Circle((central[0],central[1]), radius=raio_uav, color='b', fill=False, linestyle='dotted')
    # )

    # cont = 0
    # for i in range(0, len(point_value), 2):
    #     ax.annotate(str(value[cont]), xy=(point_value[i], point_value[i+1]))
    #     cont=cont+1

    plt.xlim([0,lim[0]])
    plt.ylim([0,lim[1]])
    plt.xlabel('X (m)')
    plt.ylabel('Y (m)')
    # plt.title("Cenario @"+'etapa/'+time+"s")

    lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.23), fancybox=True, shadow=True, ncol=5)

    plt.savefig(main_path+'e'+time+'uavloc.svg', bbox_extra_artists=(lgd,), bbox_inches='tight')
    plt.savefig(main_path+'e'+time+'uavloc.eps', bbox_extra_artists=(lgd,), bbox_inches='tight')
    plt.savefig(main_path+'e'+time+'uavloc.png', bbox_extra_artists=(lgd,), bbox_inches='tight')
    # plt.savefig('teste.svg')
    plt.close()
