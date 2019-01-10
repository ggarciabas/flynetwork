
# -*- coding: UTF-8 -*-
# libraries and data
import sys
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from matplotlib import colors as mcolors
import sys
import glob

colors = dict(mcolors.BASE_COLORS, **mcolors.CSS4_COLORS)

path = sys.argv[1]
main_path = "./scratch/flynetwork/data/output/"+path
etapa = sys.argv[2]
iteracao = sys.argv[3]
rcob = sys.argv[4]

for arquivo in glob.glob(main_path+'/'+'etapa/'+etapa+'/da_loc_*'+iteracao+'.txt'):
    print arquivo
    try:
        f_cen = open(arquivo,'r')
    except IOError:
        exit()
    line = f_cen.readline().strip()
    lim = [float(x) for x in line.split(',')]
    # temperatura
    line = f_cen.readline().strip()
    temp = [float(x) for x in line.split(',')]
    # central
    line = f_cen.readline().strip()
    central = [float(x) for x in line.split(',')]
    # posicao atual da localizacao
    line = f_cen.readline().strip()
    loc = [float(x) for x in line.split(',')] # read [x y x y x y] sequence of Locations
    # posicao antiga da localizacao
    line = f_cen.readline().strip()
    loc_last = [float(x) for x in line.split(',')] # read [x y x y x y] sequence of Locations
    # posicao do pai da localizacao
    line = f_cen.readline().strip()
    father = [float(x) for x in line.split(',')] # read [x y x y x y] sequence of Locations
    f_cen.close()

    f_cen = open(main_path+'/'+'etapa/'+etapa+'/client.txt','r')
    # read CLIENTS
    line = f_cen.readline().strip()
    cli = [x for x in line.split(',')] 
    f_cen.close()

    fig = plt.figure()
    ax = fig.add_subplot(111)

    # https://matplotlib.org/api/markers_api.html points
    first = True
    for i in range(0, len(cli), 2):
        if first:
            plt.plot(float(cli[i]),float(cli[i+1]), 'b.', markersize=7.0, label="clientes")
            first = False
        else:
            plt.plot(float(cli[i]),float(cli[i+1]), 'b.', markersize=7.0)

    first = True
    for i in range(0, len(loc), 2):
        x = [loc[i],loc_last[i]]
        y = [loc[i+1],loc_last[i+1]]
        if first:
            plt.plot(loc_last[i],loc_last[i+1],'cX', markersize=7.0, label="anteior")
            plt.plot(loc[i],loc[i+1],'c^', markersize=7.0, label="atual")
            ax.add_patch(
                patches.Circle((loc[i],loc[i+1]), radius=float(rcob), color='b', fill=False, linestyle='dotted')
            )
            plt.plot(x,y,'c-', markersize=7.0)
            first = False
        else:
            plt.plot(loc_last[i],loc_last[i+1],'cX', markersize=7.0)
            plt.plot(loc[i],loc[i+1],'c^', markersize=7.0)
            ax.add_patch(
                patches.Circle((loc[i],loc[i+1]), radius=float(rcob), color='b', fill=False, linestyle='dotted')
            )
            plt.plot(x,y,'c-', markersize=7.0)
        # ax.annotate(str(int(uav[i])), xy=(uav[i+1],uav[i+2]+10))
        # ax.annotate(str(int(loc[i])), xy=(loc[i+1],loc[i+2]-10))

    first = True
    for i in range(0, len(loc), 2):
        x = [loc[i],father[i]]
        y = [loc[i+1],father[i+1]]
        if first:
            # plt.plot(father[i],father[i+1],'rD', markersize=7.0, label="pai")
            plt.plot(x,y,'r-', markersize=7.0)
            first = False
        else:
            # plt.plot(father[i],father[i+1],'rD', markersize=7.0)
            plt.plot(x,y,'r-', markersize=7.0)
        # ax.annotate(str(int(uav[i])), xy=(uav[i+1],uav[i+2]+10))
        # ax.annotate(str(int(loc[i])), xy=(loc[i+1],loc[i+2]-10))


    plt.plot(central[0],central[1],'g*', markersize=7.0, label="central")

    plt.xlim([0,lim[0]])
    plt.ylim([0,lim[1]])
    plt.xlabel('X (m)')
    plt.ylabel('Y (m)')
    plt.title("Cenario @"+'etapa/'+etapa+"s Temp.:"+str(temp[0])+ "Iter:"+iteracao)

    lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.23), fancybox=True, shadow=True, ncol=5)

    plt.savefig(main_path+'/'+'etapa/'+etapa+'/da_loc_'+'{:015}'.format(int(iteracao))+'.svg', bbox_extra_artists=(lgd,), bbox_inches='tight')
    plt.savefig(main_path+'/'+'etapa/'+etapa+'/da_loc_'+'{:015}'.format(int(iteracao))+'.eps', bbox_extra_artists=(lgd,), bbox_inches='tight')
    plt.savefig(main_path+'/'+'etapa/'+etapa+'/da_loc_'+'{:015}'.format(int(iteracao))+'.png', bbox_extra_artists=(lgd,), bbox_inches='tight')
    # plt.savefig('teste.svg')
