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

        f_file = open(main_path+str(time)+"/exaustive.txt", 'w')
        for k in range(0,K,1):
            f_file.write(str(Yj[k][0])+','+str(Yj[k][1])+'\n')
            for u in uav_user_connected[k]:
                f_file.write(str(id[u])+' ')
            f_file.write('\n')
        f_file.close()

        # criar as matrizes do exaustivo e do proposto
        # pegar o maior valor para inserir no limite da barra para que fique padronizada e permita comparar melhor

        # print exaustive heatmap
        plt.clf()
        cmap = sns.cubehelix_palette(50, hue=0.05, rot=0, light=0.9, dark=0, as_cmap=True)
        # Create a dataset
        if teste:
            print data_bij[0]
        df_bij = pd.DataFrame(data_bij, index=locs)
        if teste:
            print df_bij[0]
        # Default heatmap: just a visualization of this square matrix
        sns.heatmap(df_bij, cmap=cmap, vmin=0, vmax=1)
        plt.ylabel("Localizacao") # TODO: validar se é assim ou o contrário!
        plt.xlabel("UAV")
        # general title
        plt.title("Custo final exaustivo", fontsize=13, fontweight=0, color='black', style='italic')
        plt.savefig(main_path+time+'/exaustive.svg')
        plt.savefig(main_path+time+'/exaustive.png')
        plt.savefig(main_path+time+'/exaustive.eps')
        plt.clf()
