# -*- coding: UTF-8 -*-
# libraries and data
import sys
import numpy as np
import seaborn as sns
from random import shuffle
import glob
import os
import matplotlib.pyplot as plt
import pandas as pd

teste = True
if sys.argv[1] == "False":
    teste = False
scenario = sys.argv[2]

custos = ["custo_1", "custo_2", "custo_3"]
data_c = {"Custo 1":[],"Custo 2":[], "Custo 3":[]}
index_c = []
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
    data = {"Exaustivo":[], "Sequencial":[], "Aleatório":[]}
    c_name = ""
    if custo == "custo_1":
        c_name = "Custo 1"
    elif custo == "custo_2":
        c_name = "Custo 2"
    elif custo == "custo_3":
        c_name = "Custo 3"
    data[c_name] = []
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
            print 'Uavs Ids'
            print uavs_id
            print 'Locs ids'
            print locs_id

        min_conf=range(0, len(locs_id))
        min_value=9999.0
        if teste:
            print "Min conf: "+str(min_conf)+" minV: "+str(min_value)

        # thanks to: https://www.geeksforgeeks.org/write-a-c-program-to-print-all-permutations-of-a-given-string/
        def permute(uav_loc, start, end): # each positionof uav_loc corresponds to a UAV and each value in any position corresponds to the location, this relates an UAV to a Location
            global min_value
            global min_conf
            if start==end:
                value = 0.0
                p=0
                for l in uav_loc:
                    value = value + data_bij[uavs_id[p]][l]
                    p=p+1
                if value < min_value:
                    min_value = value
                    min_conf = uav_loc[:] # if do not use [:], both uav_loc and min_conf will corresponds to the same pointer in memory!
                if teste:
                    print 'Uav Loc: '+str(uav_loc)+' > '+str(value)
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
        f_file_s = open(main_path+str(time)+"/sequential.txt", 'w')
        f_file_a = open(main_path+str(time)+"/random.txt", 'w')

        f_file.write(str(uavs_id[0]))
        f_file_a.write(str(uavs_id[0]))
        f_file_s.write(str(uavs_id[0]))
        for u in uavs_id[1:]: ## imprime ignorando a primeira posicao
            f_file.write(","+str(u))
            f_file_a.write(","+str(u))
            f_file_s.write(","+str(u))
        f_file.write('\n')
        f_file_a.write('\n')
        f_file_s.write('\n')
        f_file.write(str(locs_id[0]))
        f_file_a.write(str(locs_id[0]))
        f_file_s.write(str(locs_id[0]))
        for l in locs_id[1:]: ## imprime ignorando a primeira posicao
            f_file.write(","+str(l))
            f_file_a.write(","+str(l))
            f_file_s.write(","+str(l))
        f_file.write('\n')
        f_file_a.write('\n')
        f_file_s.write('\n')
        f_file.write(str(locs_id[min_conf[0]]))
        for mc in min_conf[1:]:## imprime ignorando a primeira posicao
            f_file.write(","+str(locs_id[mc]))
        f_file.write('\n')
        f_file.write(str(min_conf)) # para validar se esta gravando corretamente
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
        p_conf = []
        for key, values in data_mij.iteritems():
            l=0
            for v in values:
                c_prop[c][l] = v*data_bij[uavs_id[c]][l]
                if v == 1.0:
                    p_conf.append(l)
                l=l+1
            c=c+1

        c_seq = np.zeros((len(uavs_id),len(locs_id)))
        s_conf = []
        for i in range(0, len(uavs_id)):
            s_conf.append(i)
            c_seq[i][i] = data_bij[uavs_id[i]][i]
        f_file_s.write(str(locs_id[s_conf[0]]))
        for i in s_conf[1:]:
            f_file_s.write(","+str(locs_id[i]))
        f_file_s.write('\n')

        c_ale = np.zeros((len(uavs_id),len(locs_id)))
        ale_conf = range(0, len(locs_id))
        shuffle(ale_conf)
        c=0
        for i in ale_conf:
            c_ale[c][i] = data_bij[uavs_id[c]][i]
            c=c+1
        f_file_a.write(str(locs_id[ale_conf[0]]))
        for i in ale_conf[1:]:
            f_file_a.write(","+str(locs_id[i]))
        f_file_a.write('\n')

        f_file_s.close()
        f_file_a.close()

        if teste:
            print "C_EX: "+str(c_exaustive)+" \nC_PRO: " +str(c_prop)+" \nC_SEQ: " +str(c_seq)+" \nC_ALE: " +str(c_ale)

        # pegar o maior valor para inserir no limite da barra para que fique padronizada e permita comparar melhor
        m_e = c_exaustive.max()
        m_p = c_prop.max()
        m_s = c_seq.max()
        m_a = c_ale.max()
        m_v = max(m_e, m_p, m_s, m_a)

        # print exaustive heatmap
        plt.clf()
        cmap = sns.cubehelix_palette(50, hue=0.05, rot=0, light=0.9, dark=0, as_cmap=True)
        # Create a dataset
        df_exh = pd.DataFrame(c_exaustive)
        # Default heatmap: just a visualization of this square matrix
        sns.heatmap(df_exh, cmap=cmap, vmin=0, vmax=m_v)
        plt.xlabel(u"Localização")
        plt.ylabel("UAV")
        # general title
        plt.title("Custo final exaustivo", fontsize=13, fontweight=0, color='black', style='italic')
        plt.savefig(main_path+str(time)+'/exaustive_'+str(custo)+'.svg')
        plt.savefig(main_path+str(time)+'/exaustive_'+str(custo)+'.png')
        plt.savefig(main_path+str(time)+'/exaustive_'+str(custo)+'.eps')

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
        plt.savefig(main_path+str(time)+'/'+str(custo)+'.svg')
        plt.savefig(main_path+str(time)+'/'+str(custo)+'.png')
        plt.savefig(main_path+str(time)+'/'+str(custo)+'.eps')
        plt.clf()

        # print sequential heatmap
        plt.clf()
        cmap = sns.cubehelix_palette(50, hue=0.05, rot=0, light=0.9, dark=0, as_cmap=True)
        # Create a dataset
        df_seq = pd.DataFrame(c_seq)
        # Default heatmap: just a visualization of this square matrix
        sns.heatmap(df_seq, cmap=cmap, vmin=0, vmax=m_v)
        plt.xlabel(u"Localização")
        plt.ylabel("UAV")
        # general title
        plt.title("Custo final sequencial", fontsize=13, fontweight=0, color='black', style='italic')
        plt.savefig(main_path+str(time)+'/seq_'+str(custo)+'.svg')
        plt.savefig(main_path+str(time)+'/seq_'+str(custo)+'.png')
        plt.savefig(main_path+str(time)+'/seq_'+str(custo)+'.eps')
        plt.clf()

        # print aletorio heatmap
        plt.clf()
        cmap = sns.cubehelix_palette(50, hue=0.05, rot=0, light=0.9, dark=0, as_cmap=True)
        # Create a dataset
        df_ale = pd.DataFrame(c_ale)
        # Default heatmap: just a visualization of this square matrix
        sns.heatmap(df_ale, cmap=cmap, vmin=0, vmax=m_v)
        plt.xlabel(u"Localização")
        plt.ylabel("UAV")
        # general title
        plt.title(u"Custo final aleatório", fontsize=13, fontweight=0, color='black', style='italic')
        plt.savefig(main_path+str(time)+'/ale_'+str(custo)+'.svg')
        plt.savefig(main_path+str(time)+'/ale_'+str(custo)+'.png')
        plt.savefig(main_path+str(time)+'/ale_'+str(custo)+'.eps')
        plt.clf()

        data["Exaustivo"].append(c_exaustive.sum())
        # data["Proposto"].append(c_prop.sum()-c_exaustive.sum()) # para apresentar a diferença
        data[c_name].append(c_prop.sum())
        data["Sequencial"].append(c_seq.sum())
        data["Aleatório"].append(c_ale.sum())

        data_c[c_name].append(c_prop.sum())

        if c_prop.sum() < c_exaustive.sum():
            print 'ERROO'
            input()

        if teste:
            print '===================\n\n'

        index_c = list_folder[:]


    # plot comparation all times [https://pandas.pydata.org/pandas-docs/stable/generated/pandas.DataFrame.plot.bar.html]
    # stacked=True
    index = list_folder[:]
    df = pd.DataFrame(data, index=index)
    # ax = df.plot.bar(stacked=True, rot=0)
    ax = df.plot.bar(rot=0)

    ax.set_title('Comparativo do custo total')
    ax.set_ylabel('Custo total')
    ax.set_xlabel('Tempo (s)')

    plt.savefig(main_path+'/exh_'+str(custo)+'.svg')
    plt.savefig(main_path+'/exh_'+str(custo)+'.png')
    plt.savefig(main_path+'/exh_'+str(custo)+'.eps')
    plt.clf()

# plot comparation all cost
# df = pd.DataFrame(data_c, index=index_c)
# ax = df.plot.bar(rot=0)
#
# ax.set_title('Comparativo do custo total (sem sentido, bijs são diferentes)')
# ax.set_ylabel('Custo total')
# ax.set_xlabel('Tempo (s)')
#
# plt.savefig("./output/"+scenario+'/custos_comparativo.svg')
# plt.savefig("./output/"+scenario+'/custos_comparativo.png')
# plt.savefig("./output/"+scenario+'/custos_comparativo.eps')
# plt.clf()
