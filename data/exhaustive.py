# -*- coding: UTF-8 -*-
# libraries and data
import sys
import numpy as np
import seaborn as sns
import glob
import os
import matplotlib.pyplot as plt
import pandas as pd

teste = True
if sys.argv[1] == "False":
    teste = False
scenario = sys.argv[2]

custos = ["custo_1"] #, "custo_2", "custo_3"]
for custo in custos:
    main_path = "./output/"+scenario+"/"+custo+"/"
    list_folder = []
    if len(sys.argv) == 4: # folder number
        list_folder.append(int(sys.argv[3]))
    else:
        for folder_name in glob.glob(main_path+'*/'):
            list_folder.append(int(os.path.dirname(folder_name).split('/')[-1]))
    if teste:
        print (list_folder)
    list_folder = np.array(list_folder)
    list_folder.sort()

    for time in list_folder:
        # read bij
        try:
            file = open(main_path+str(time)+"/bij.txt", 'r')
        except IOError:
            exit()
        line = file.readline().strip()
        uavs_id = [int(x) for x in line.split(',')]
        line = file.readline().strip()
        locs_id = [int(x) for x in line.split(',')]
        data_bij = {}
        cont = 0
        for line in file:
            data_bij[uavs_id[cont]] = [float(x) for x in line.split(',')] # read row from file
            cont = cont + 1
        file.close()

        try:
            file = open(main_path+str(time)+"/mij.txt", 'r')
        except IOError:
            exit()
        line = file.readline().strip()
        line = file.readline().strip()
        line = file.readline().strip()
        data_mij = {}
        cont = 0
        for line in file:
            data_mij[uavs_id[cont]] = [float(x) for x in line.split(',')] # read row from file
            cont = cont + 1
        file.close()

        if teste:
            print "Data_bij"
            print data_bij

        min_conf=locs_id[:]
        min_value=9999.0
        if teste:
            print "Min conf: "+str(min_conf)+" minV: "+str(min_value)

        # thanks to: https://www.geeksforgeeks.org/write-a-c-program-to-print-all-permutations-of-a-given-string/
        def permute(uav_loc, start, end): # each positionof uav_loc corresponds to a UAV and each value in any position corresponds to the location, this relates an UAV to a Location
            global min_value
            global min_conf
            if start==end:
                value = 0.0
                c=0
                for l in uav_loc:
                    value = value + data_bij[uavs_id[c]][l]
                    c=c+1
                if value < min_value:
                    if teste:
                        print "LOWER >> mv: "+str(min_value)+" v: "+str(value)
                    min_value = value
                    min_conf = uav_loc[:] # if do not use [:], both uav_loc and min_conf will corresponds to the same pointer in memory!
            else:
                for i in xrange(start,end+1):
                    uav_loc[start], uav_loc[i] = uav_loc[i], uav_loc[start]
                    permute(uav_loc, start+1, end)
                    uav_loc[start], uav_loc[i] = uav_loc[i], uav_loc[start] # backtrack

        permute (min_conf[:], 0, len(min_conf)-1)

        if teste:
            print "Min conf: "+str(min_conf)+" minV: "+str(min_value)

        def toString(List):
            return ','.join(List)

        f_file = open(main_path+str(time)+"/exaustive.txt", 'w')
        f_file.write(str(uavs_id[0]))
        for u in uavs_id[1:]: ## imprime ignorando a primeira posicao
            f_file.write(","+str(u))
        f_file.write('\n')
        f_file.write(str(locs_id[0]))
        for l in locs_id[1:]: ## imprime ignorando a primeira posicao
            f_file.write(","+str(l))
        f_file.write('\n')
        f_file.write(str(min_conf[0]))
        for mc in min_conf[1:]: ## imprime ignorando a primeira posicao
            f_file.write(","+str(mc))
        f_file.write('\n')
        f_file.close()

        # criar as matrizes do exaustivo e do proposto
        c_exaustive = np.zeros((len(uavs_id),len(locs_id)))
        c=0
        for l in min_conf:
            c_exaustive[c][l] = data_bij[uavs_id[c]][l]
            c=c+1

        c_prop = np.zeros((len(uavs_id),len(locs_id)))
        if teste:
            print "MIJ: "+str(data_mij)
            print "BIJ: "+str(data_bij)
        c=0
        for key, values in data_mij.iteritems():
            l=0
            for v in values:
                c_prop[c][l] = v*data_bij[uavs_id[c]][l]
                l=l+1
            c=c+1

        if teste:
            print "C_EX: "+str(c_exaustive)+" \nC_PRO: " +str(c_prop)

        # pegar o maior valor para inserir no limite da barra para que fique padronizada e permita comparar melhor
        m_e = c_exaustive.max()
        m_p = c_prop.max()
        m_v = max(m_e, m_p)

        # print exaustive heatmap
        plt.clf()
        cmap = sns.cubehelix_palette(50, hue=0.05, rot=0, light=0.9, dark=0, as_cmap=True)
        # Create a dataset
        if teste:
            print c_exaustive
        df_exh = pd.DataFrame(c_exaustive)
        if teste:
            print df_exh
        # Default heatmap: just a visualization of this square matrix
        sns.heatmap(df_exh, cmap=cmap, vmin=0, vmax=m_v)
        plt.xlabel(u"Localização")
        plt.ylabel("UAV")
        # general title
        plt.title("Custo final exaustivo", fontsize=13, fontweight=0, color='black', style='italic')
        plt.savefig(main_path+str(time)+'/exaustive.svg')
        plt.savefig(main_path+str(time)+'/exaustive.png')
        plt.savefig(main_path+str(time)+'/exaustive.eps')

        # print propose heatmap
        plt.clf()
        cmap = sns.cubehelix_palette(50, hue=0.05, rot=0, light=0.9, dark=0, as_cmap=True)
        # Create a dataset
        df_prop = pd.DataFrame(c_prop)
        # Default heatmap: just a visualization of this square matrix
        sns.heatmap(df_prop, cmap=cmap, vmin=0, vmax=m_v)
        plt.xlabel(u"Localização")
        plt.ylabel("UAV")
        # general title
        plt.title("Custo final algoritmo proposto", fontsize=13, fontweight=0, color='black', style='italic')
        plt.savefig(main_path+str(time)+'/proposed.svg')
        plt.savefig(main_path+str(time)+'/proposed.png')
        plt.savefig(main_path+str(time)+'/proposed.eps')
        plt.clf()

        # bar plot > https://matplotlib.org/gallery/lines_bars_and_markers/bar_stacked.html#sphx-glr-gallery-lines-bars-and-markers-bar-stacked-py
        ind = np.arange(1)
        width = 0.35

        p1 = plt.bar(ind, c_exaustive.sum(), width)
        p2 = plt.bar(ind, c_prop.sum()-c_exaustive.sum(), width, bottom=c_exaustive.sum())

        plt.ylabel('Custo total')
        plt.title('Comparativo do custo total')
        plt.legend((p1[0], p2[0]), ('Exaustivo', 'Proposto'))

        plt.savefig(main_path+str(time)+'/bar_exh_prop.svg')
        plt.savefig(main_path+str(time)+'/bar_exh_prop.png')
        plt.savefig(main_path+str(time)+'/bar_exh_prop.eps')
        plt.clf()
