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

custos = ["custo_1", "custo_2", "custo_3", "custo_4"]
c_name = ["Custo 1", "Custo 2", "Custo 3", "Custo 4"]
data_c = {"Custo 1":[], "Custo 2":[], "Custo 3":[], "Custo 4":[]}

## Custo
for custo in range(0,len(custos)):
    main_path = "./output/"+scenario+"/"+custos[custo]+"/"
    time_folders = []
    if len(sys.argv) == 4: # folder number
        time_folders.append(int(sys.argv[3]))
    else:
        for folder_name in glob.glob(main_path+'etapa/*/'):
            if teste:
                print "folder_name: "+str(folder_name)
            time_folders.append(int(os.path.dirname(folder_name).split('/')[-1]))
    time_folders = np.array(time_folders)
    time_folders.sort()
    t_cont = 0
    for time in time_folders[:-1]: # ignora o ultimo tempo
        if teste:
            print "Tempo: " + str(time)
        try:
            f_mij = open(main_path+'etapa/'+str(time)+"/f_mij.txt", 'r')
        except IOError:
            print "Failed f_mij"
            exit()
        uav_loc = [] # each pos corresponds to an UAV, each value the location
        line = f_mij.readline().strip() # ignore temp
        line = f_mij.readline().strip() # ignore UAVs ids
        line = f_mij.readline().strip()
        locs_id = [int(x) for x in line.split(',')]
        for line in f_mij:
            cont = 0
            for x in line.split(','):
                if int(x) == 1:
                    uav_loc.append(locs_id[cont])
                    break
                cont = cont + 1
        f_mij.close()

        if teste:
            print "UavLoc: "+str(uav_loc)+" locs_id: "+str(locs_id)

        # ler total de energia disponível no Uav
        # obs. este total depende da execucao (avaliado para cada custo), assim deve ser analisado também outros fatores, como desperdício, por exemplo
        # uav_energy.txt
        try:
            f_energy = open(main_path+'etapa/'+str(time)+"/uav_energy.txt", 'r')
        except IOError:
            print "Failed uav_energy"
            exit()
        uav_energy = []
        for line in f_energy:
            values = [x for x in line.split(',')] # UAV_ID, ATUAL_ENERGY, INITIAL_ENERGY
            uav_energy.append(float(values[1]))
        f_energy.close()

        # ler localizacoes
        try:
            f_location = open(main_path+'etapa/'+str(time)+"/location_client.txt", 'r')
        except IOError:
            print "Failed location_client"
            exit()
        loc_cons = {}
        loc_cli = {}
        for line in f_location:
            values = [x for x in line.split(',')] # LOC_ID, TOTAL_CLI, TOTAL_CONSUMPTION
            loc_cons[str(values[0])] = float(values[2])
            loc_cli[str(values[0])] = int(values[1])
        f_location.close()

        if teste:
            print "Cons: "+str(loc_cons)+" Cli: "+str(loc_cli)

        # verificar se o UAV finaliza sua energia antes do proximo ciclo
        # lembrando que o consumo da localizacao é por segundo!
        # calcular o tempo para o proximo ciclo usando o vetor de tempos time_folders
        diff_time = time_folders[t_cont+1] - time

        total_cli = 0
        for i in uav_loc:
            total_cli = total_cli + loc_cli.get(str(i))
        data_c[c_name[custo]].append([time, total_cli]) # time, total_cli

        atual_cli = 0
        c=0
        for i in uav_loc:
            atual_cli = atual_cli + loc_cli.get(str(i))
            if loc_cons.get(str(i)) > 0:
                sec = uav_energy[c]/loc_cons.get(str(i)) # resist time in seconds
                if teste:
                    print "Diff: "+str(diff_time)+" - Sec: " +str(sec)
                if sec < diff_time:
                    atual_cli = atual_cli - loc_cli.get(str(i))
                    data_c[c_name[custo]].append([time+sec, atual_cli])
            c=c+1

        t_cont = t_cont + 1


if teste:
    print data_c

# plot
plt.clf()

fig = plt.figure()
ax = fig.add_subplot(111)

# create a color palette
palette = plt.get_cmap('Set1')
num=0
for custo,value in data_c.iteritems():
    num+=1
    # Plot the lineplot
    value=np.array(value)
    plt.plot(value[:,0], value[:,1], marker='', color=palette(num), linewidth=2.4, alpha=0.9, label=custo)
plt.xlabel(u"Tempo (s)")
plt.ylabel("Total de clientes")
# general title
plt.title(u"Quantidade de clientes que podem estar ativos", fontsize=13, fontweight=0, color='black', style='italic')

lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.17), fancybox=True, shadow=True, ncol=5)

plt.savefig("./output/"+scenario+'/connected_cli.svg')
plt.savefig("./output/"+scenario+'/connected_cli.png')
plt.savefig("./output/"+scenario+'/connected_cli.eps')
plt.clf()