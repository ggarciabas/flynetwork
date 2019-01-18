
# -*- coding: UTF-8 -*-
# libraries and data
import sys
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from matplotlib import colors as mcolors
import sys
import glob
import pandas as pd

path = sys.argv[1]
main_path = "./scratch/flynetwork/data/output/"+path
etapa = sys.argv[2]
da = "cpp"

print(plt.rcParams.get('figure.figsize'))

for arquivo in glob.glob(main_path+'/'+'etapa/'+etapa+'/da_loc_'+da+'_*'+iteracao+'.txt'):
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
    data = {}
    line = f_cen.readline().strip()
    data["C"] = [float(x) for x in line.split(',')]
    line = f_cen.readline().strip()
    data["N"] = [float(x) for x in line.split(',')]
    line = f_cen.readline().strip()
    data["Plj"] = [float(x) for x in line.split(',')]
    f_cen.close()

    lId = np.arange(0,len(loc),1);

    f_cen = open(main_path+'/'+'etapa/'+etapa+'/client.txt','r')
    # read CLIENTS
    line = f_cen.readline().strip()
    cli = [x for x in line.split(',')]
    f_cen.close()

    # fig = plt.figure(figsize=(16,6))
    fig = plt.figure(figsize=(15,4))
    ax0 = fig.add_subplot(121)

    # https://matplotlib.org/api/markers_api.html points
    first = True
    for i in range(0, len(cli), 2):
        if first:
            ax0.plot(float(cli[i]),float(cli[i+1]), 'b.', markersize=7.0, label="clientes")
            first = False
        else:
            ax0.plot(float(cli[i]),float(cli[i+1]), 'b.', markersize=7.0)

    first = True
    c = 0;
    for i in range(0, len(loc), 2):
        x = [loc[i],loc_last[i]]
        y = [loc[i+1],loc_last[i+1]]
        if first:
            ax0.plot(loc_last[i],loc_last[i+1],'cX', markersize=7.0, label="anteior")
            ax0.plot(loc[i],loc[i+1],'c^', markersize=7.0, label="atual")
            ax0.add_patch(
                patches.Circle((loc[i],loc[i+1]), radius=float(rcob), color='b', fill=False, linestyle='dotted')
            )
            ax0.plot(x,y,'c-', markersize=7.0)
            first = False
        else:
            ax0.plot(loc_last[i],loc_last[i+1],'cX', markersize=7.0)
            ax0.plot(loc[i],loc[i+1],'c^', markersize=7.0)
            ax0.add_patch(
                patches.Circle((loc[i],loc[i+1]), radius=float(rcob), color='b', fill=False, linestyle='dotted')
            )
            ax0.plot(x,y,'c-', markersize=7.0)
        
        ax0.annotate(str(int(lId[c])), xy=(loc[i],loc[i+1]-5))
        c=c+1

    first = True
    for i in range(0, len(loc), 2):
        x = [loc[i],father[i]]
        y = [loc[i+1],father[i+1]]
        if first:
            # plt.plot(father[i],father[i+1],'rD', markersize=7.0, label="pai")
            ax0.plot(x,y,'r-', markersize=7.0)
            first = False
        else:
            # plt.plot(father[i],father[i+1],'rD', markersize=7.0)
            ax0.plot(x,y,'r-', markersize=7.0)
        # ax.annotate(str(int(uav[i])), xy=(uav[i+1],uav[i+2]+10))
        # ax.annotate(str(int(loc[i])), xy=(loc[i+1],loc[i+2]-10))


    ax0.plot(central[0],central[1],'g*', markersize=7.0, label="central")

    # plt.xlim([0,lim[0]])
    # plt.ylim([0,lim[1]])
    # plt.xlabel('X (m)')
    # plt.ylabel('Y (m)')
    # plt.title("Cenario @"+'etapa/'+etapa+"s Temp.:"+str(temp[0])+ " Iter:"+iteracao+" TLoc: "+ tLoc)

    ax0.set_title("Cenario @"+'etapa/'+etapa+"s Temp.:"+str(temp[0])+ " Iter:"+iteracao+" TLoc: "+ tLoc)
    ax0.set_ylabel('Y (m)')
    ax0.set_xlabel('X (m)')
    ax0.set_xlim([0,lim[0]])
    ax0.set_ylim([0,lim[1]])

    lgd = ax0.legend(loc='upper center', bbox_to_anchor=(0.5, 1.23), fancybox=True, shadow=True, ncol=5)

    # plt.savefig(main_path+'/'+'etapa/'+etapa+'/da_loc_'+da+'_{:015}'.format(int(iteracao))+'.svg', bbox_extra_artists=(lgd,), bbox_inches='tight')
    # plt.savefig(main_path+'/'+'etapa/'+etapa+'/da_loc_'+da+'_{:015}'.format(int(iteracao))+'.eps', bbox_extra_artists=(lgd,), bbox_inches='tight')
    plt.savefig(main_path+'/'+'etapa/'+etapa+'/da_loc_scenario'+da+'_{:015}'.format(int(iteracao))+'.png', bbox_extra_artists=(lgd,), bbox_inches='tight')
    # plt.savefig('teste.svg')

    
    fig.set_size_inches(16, 6, forward=False)

    # All
    print data
    df = pd.DataFrame(data) #, index=lId)
    print df
    ax1 = fig.add_subplot(122)
    df.plot.bar(ax=ax1)
    ax1.set_title(u'Informações da Localização')
    ax1.set_ylabel('0-1')
    ax1.set_xlabel(u'Localizações')
    # ax1.set_ylim([0,1])
    plt.tight_layout()
    plt.savefig(main_path+'/'+'etapa/'+etapa+'/da_loc_'+da+'_{:015}'.format(int(iteracao))+'.png')
    plt.clf()

    
    ax = df.plot.bar(rot=0)

    ax.set_title(u'Informações da Localização')
    ax.set_ylabel('0-1')
    ax.set_xlabel(u'Localizações')
    # ax.set_ylim([0,1])

    # plt.savefig(main_path+'/'+'etapa/'+etapa+'/da_loc_hsit_'+da+'_{:015}'.format(int(iteracao))+'.svg')
    # plt.savefig(main_path+'/'+'etapa/'+etapa+'/da_loc_hsit_'+da+'_{:015}'.format(int(iteracao))+'.eps')
    plt.savefig(main_path+'/'+'etapa/'+etapa+'/da_loc_hist_'+da+'_{:015}'.format(int(iteracao))+'.png')
    plt.clf()