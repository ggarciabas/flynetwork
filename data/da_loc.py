
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

da = "cpp"

c = sys.argv[1]
e = sys.argv[2]
m = sys.argv[3]
t = sys.argv[4]
i = sys.argv[5]

def da_loc (custo, etapa, main_path, teste, it=-1):
    # print "DA LOc - new Graphic"
    f_cen = open(main_path+'/'+str(custo)+'/etapa/'+str(etapa)+'/client.txt','r')
    line = f_cen.readline().strip()
    cli = [x for x in line.split(',')]
    f_cen.close()
    arquivos = glob.glob(main_path+'/'+str(custo)+'/etapa/'+str(etapa)+'/da_loc_'+da+'_*.txt')
    arquivos = np.array(arquivos)
    arquivos.sort()
    if int(it) != -1:
        arquivos = [arquivos[int(it)-1]]
    for i in np.arange(0, len(arquivos), 10):
        arquivo = arquivos[i]
        try:
            f_cen = open(arquivo,'r')
        except IOError:
            exit()
        # iteracao do DA
        line = f_cen.readline().strip()
        iteracao = [float(x) for x in line.split(',')]
        # distance uavcob
        line = f_cen.readline().strip()
        uavcob = [float(x) for x in line.split(',')]
        uavcob = uavcob[0]
        # distance max_antena_cli
        line = f_cen.readline().strip()
        maxclicob = [float(x) for x in line.split(',')]
        maxclicob = maxclicob[0]
        # raio de cobertura
        line = f_cen.readline().strip()
        rcob = [float(x) for x in line.split(',')]
        rcob = [max(rcob)]
        # limits
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
        # line = f_cen.readline().strip()
        # data["Plj"] = [float(x) for x in line.split(',')]
        # connected
        line = f_cen.readline().strip()
        connected = [int(x) for x in line.split(',')]
        line = f_cen.readline().strip()
        data["Wij"] = [float(x) for x in line.split(',')]
        data_cli = {}
        line = f_cen.readline().strip()
        data_cli["DR"] = [float(x) for x in line.split(',')]        
        f_cen.close()        

        lId = np.arange(0,len(loc),1);        

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
                    patches.Circle((loc[i],loc[i+1]), radius=float(uavcob), color='r', fill=False, linestyle='dashed')
                )
                ax0.add_patch(
                    patches.Circle((loc[i],loc[i+1]), radius=float(rcob[0]), color='b', fill=False, linestyle='dotted')
                )
                ax0.add_patch(
                    patches.Circle((loc[i],loc[i+1]), radius=float(maxclicob), color='g', fill=False, linestyle='dotted')
                )
                ax0.plot(x,y,'c-', markersize=7.0)
                first = False
            else:
                ax0.plot(loc_last[i],loc_last[i+1],'cX', markersize=7.0)
                ax0.plot(loc[i],loc[i+1],'c^', markersize=7.0)
                ax0.add_patch(
                    patches.Circle((loc[i],loc[i+1]), radius=float(uavcob), color='r', fill=False, linestyle='dashed')
                )
                ax0.add_patch(
                    patches.Circle((loc[i],loc[i+1]), radius=float(rcob[0]), color='b', fill=False, linestyle='dotted')
                )
                ax0.add_patch(
                    patches.Circle((loc[i],loc[i+1]), radius=float(maxclicob), color='g', fill=False, linestyle='dotted')
                )
                ax0.plot(x,y,'c-', markersize=7.0)
            
            ax0.annotate(str(int(lId[c])), xy=(loc[i],loc[i+1]-5))
            c=c+1

        first = True
        l = 0
        for i in range(0, len(loc), 2):
            x = [loc[i],father[i]]
            y = [loc[i+1],father[i+1]]
            if int(connected[l]) == 1:
                if first:
                    ax0.plot(x,y,'r-', markersize=7.0)
                    first = False
                else:
                    ax0.plot(x,y,'r-', markersize=7.0)
            l = l + 1

        ax0.plot(central[0],central[1],'g*', markersize=7.0, label="central")

        ax0.set_title("Cenario @"+'etapa/'+str(etapa)+"s Temp.:"+str(temp[0])+ " Iter:"+str(iteracao[0])+" CobUav: "+ str(uavcob))
        ax0.set_ylabel('Y (m)')
        ax0.set_xlabel('X (m)')
        ax0.set_xlim([0,lim[0]])
        ax0.set_ylim([0,lim[1]])

        lgd = ax0.legend(loc='upper center', bbox_to_anchor=(0.5, 1.23), fancybox=True, shadow=True, ncol=5)

        # plt.savefig(main_path+'/'+str(custo)+'/etapa/'+str(etapa)+'/da_loc_'+da+'_{:015}'.format(int(iteracao[0]))+'.svg', bbox_extra_artists=(lgd,), bbox_inches='tight')
        # plt.savefig(main_path+'/'+str(custo)+'/etapa/'+str(etapa)+'/da_loc_'+da+'_{:015}'.format(int(iteracao[0]))+'.eps', bbox_extra_artists=(lgd,), bbox_inches='tight')
        plt.savefig(main_path+'/'+str(custo)+'/etapa/'+str(etapa)+'/da_loc_scenario'+da+'_{:015}'.format(int(iteracao[0]))+'.png', bbox_extra_artists=(lgd,), bbox_inches='tight')
        # plt.savefig('teste.svg')

        fig.set_size_inches(16, 6, forward=False)

        # All
        # print data
        df = pd.DataFrame(data) #, index=lId)
        # # print df
        ax1 = fig.add_subplot(122)
        df.plot.bar(ax=ax1)
        ax1.set_title(u'Informações da Localização')
        ax1.set_ylabel('0-1')
        ax1.set_xlabel(u'Localizações')

        # client datarate
        df_cli = pd.DataFrame(data_cli)
        # ax2 = fig.add_subplot(223)
        # df_cli.plot.bar(ax=ax2)
        # ax2.set_title(u'Informações do Cliente')
        # ax2.set_ylabel('Taxa (Mbps)')
        # ax2.set_xlabel(u'Clientes')

        plt.tight_layout()
        plt.savefig(main_path+'/'+str(custo)+'/etapa/'+str(etapa)+'/da_loc_'+da+'_{:015}'.format(int(iteracao[0]))+'.png')
        plt.clf()
        
        ax = df.plot.bar(rot=0)

        ax.set_title(u'Informações da Localização')
        ax.set_ylabel('0-1')
        ax.set_xlabel(u'Localizações')

        # plt.savefig(main_path+'/'+str(custo)+'/etapa/'+str(etapa)+'/da_loc_hist_'+da+'_{:015}'.format(int(iteracao[0]))+'.svg')
        # plt.savefig(main_path+'/'+str(custo)+'/etapa/'+str(etapa)+'/da_loc_hist_'+da+'_{:015}'.format(int(iteracao[0]))+'.eps')
        plt.savefig(main_path+'/'+str(custo)+'/etapa/'+str(etapa)+'/da_loc_hist_'+da+'_{:015}'.format(int(iteracao[0]))+'.png')
        plt.clf()

        ax = df_cli.plot.bar(rot=0)

        ax.set_title(u'Informações do Cliente')
        ax.set_ylabel('Taxa (Mbps)')
        ax.set_xlabel(u'Clientes')

        # plt.savefig(main_path+'/'+str(custo)+'/etapa/'+str(etapa)+'/client_dr_'+da+'_{:015}'.format(int(iteracao[0]))+'.svg')
        # plt.savefig(main_path+'/'+str(custo)+'/etapa/'+str(etapa)+'/client_dr_'+da+'_{:015}'.format(int(iteracao[0]))+'.eps')
        plt.savefig(main_path+'/'+str(custo)+'/etapa/'+str(etapa)+'/client_dr_'+da+'_{:015}'.format(int(iteracao[0]))+'.png')
        plt.clf()

if __name__ == "__main__":
    # print m
    da_loc(c, e, m, t, i)    