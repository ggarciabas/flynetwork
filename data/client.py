
# -*- coding: UTF-8 -*-
# libraries and data
import sys
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import colors as mcolors

colors = dict(mcolors.BASE_COLORS, **mcolors.CSS4_COLORS)


def scenario (time, main_path, teste):
    try:
        f_cen = open(main_path+'etapa/'+time+'/client.txt','r')
    except IOError:
        return
    # read CLIENTS
    line = f_cen.readline().strip()
    cli = [x for x in line.split(',')] # position that server knows
    line = f_cen.readline().strip()
    cli_ = [x for x in line.split(',')] # position real
    f_cen.close()

    fig = plt.figure()
    ax = fig.add_subplot(111)

    first = True
    app_color = {"VOICE":"darksalmon", "VIDEO":"blueviolet", "WWW":"skyblue", "NOTHING":"yellow"}
    apps = {}
    for i in range(0, len(cli_), 4): # x y login app
        if cli_[i+2] == "fixed":
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

    first = True
    for i in range(0, len(cli), 3):
        if cli[i+2] == "fixed":
            if first:
                plt.plot(float(cli[i]),float(cli[i+1]), 'bs', markersize=7.0, label="fixed")
                first = False
            else:
                plt.plot(float(cli[i]),float(cli[i+1]), 'bs', markersize=7.0)
        else:
            if first:
                plt.plot(float(cli[i]),float(cli[i+1]), 'k1', markersize=7.0, label="movel")
                first = False
            else:
                plt.plot(float(cli[i]),float(cli[i+1]), 'k1', markersize=7.0)

    plt.xlabel('X (m)')
    plt.ylabel('Y (m)')
    plt.title("Cenario @"+'etapa/'+time+"s")

    lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.23), fancybox=True, shadow=True, ncol=5)

    plt.savefig(main_path+'etapa/'+time+'/client.svg', bbox_extra_artists=(lgd,), bbox_inches='tight')
    plt.savefig(main_path+'etapa/'+time+'/client.eps', bbox_extra_artists=(lgd,), bbox_inches='tight')
    plt.savefig(main_path+'etapa/'+time+'/client.png', bbox_extra_artists=(lgd,), bbox_inches='tight')
    # plt.savefig('teste.svg')
