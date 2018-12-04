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
    for time in time_folders:
        try:
            f_removed = open(main_path+'etapa/'+str(time)+"/uav_removed_energy.txt", 'r')
            total = 0.0
            for line in f_removed:
                total = total + float(line.strip())
            data_c[c_name[custo]].append([time, total]) # time, bateria%
        except IOError:
            # não foram removidos UAVs
            data_c[c_name[custo]].append([time, 0.0]) # time, bateria%


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
plt.ylabel("Bateria (%)")
# general title
plt.title(u"Porcentagem de bateria desperdiçada ao remover UAV", fontsize=13, fontweight=0, color='black', style='italic')

lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.17), fancybox=True, shadow=True, ncol=5)

plt.savefig("./output/"+scenario+'/uav_removed_energy.svg')
plt.savefig("./output/"+scenario+'/uav_removed_energy.png')
plt.savefig("./output/"+scenario+'/uav_removed_energy.eps')
plt.clf()
