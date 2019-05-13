# -*- coding: UTF-8 -*-
# libraries and data
import matplotlib.pyplot as plt
import glob
import os
import sys
import numpy as np
import seaborn as sns
import pandas as pd 

teste = True
if sys.argv[1] == "False":
    teste = False
scenario = sys.argv[2]
main_path = "./output/"+scenario+"/"

s_time = float(sys.argv[3]) # segundos

df_main = pd.DataFrame()
for seed_name in glob.glob(main_path+'*/'):
    seed = os.path.dirname(seed_name).split('/')[-1]
    if teste:
        print "Seed: "+seed
    df_seed = pd.DataFrame()
    for protocol_name in glob.glob(main_path+seed+'/*/'):
        protocol = os.path.dirname(protocol_name).split('/')[-1]
        if int(protocol) == 1:
            protocol_str = str("OLSR")
        elif int(protocol) == 2:
            protocol_str = str("AODV")
        elif int(protocol) == 3:
            protocol_str = str("DSDV")

        for custo_name in glob.glob(main_path+seed+'/'+protocol+'/custo_*/'):
            custo = os.path.dirname(custo_name).split('/')[-1]
            c_name = int(custo[-1])
            if c_name == 1: 
                c_name = str(u"$C_1$")
            elif c_name == 2: 
                c_name = str(u"$C_2$")
            elif c_name == 4: # OBS! verificar necessidade de alteração!
                c_name = str(u"$C_3$")
            if teste:
                print "Custo: "+custo

            # structure of log file 
            # TIME UAV_ID INITIAL_E ACTUAL_E WIFI_E CLIENT_E MOVE_E HOVER_E
            # remove duplicated lines on log file
            lines_seen = set() # holds lines already seen
            for line in open(main_path+seed+'/'+protocol+'/'+custo+'/uav_energy/uav_energy.txt', "r"):
                if line not in lines_seen: # not a duplicate
                    lines_seen.add(line)

            # each line of the list is a UAV is necessary to calculate the proportion between WIFI_E and INITIAL
            df_custo = pd.DataFrame()
            for line in lines_seen:
                line_data = [x for x in line.split(' ')]
                iniE = float(line_data[2])
                remE = float(line_data[3])
                wifiE = float(line_data[4])
                moveE = float(line_data[6])
                hoverE = float(line_data[7])

                # W1: WIFI_E/(INIE-REME) relação da bateria presente no cenario!
                # W2: WIFI_E/INIE realcao da bateria inicial, independente da jogada fora
                # M1: MOVE_E/(INIE-REME)
                # M2: MOVE_E/INIE
                # H1: HOVER_E/(INIE-REME)
                # H2: HOVER_E/INIE
                # D1: REME/INIE relacao da bateria jogada fora em relacao a inicial
                # U1: (INIE-REME)/INIE relacao da bateria util no cenario
                df = pd.DataFrame({"SEED":[seed], "COST":[c_name], "W1":[wifiE/(iniE-remE)], "W2":[wifiE/iniE], "M1":[moveE/(iniE-remE)], "M2":[moveE/iniE], "H1":[hoverE/(iniE-remE)], "H2":[hoverE/iniE], "D1":[remE/iniE], "U1":[(iniE-remE)/iniE], "PROTOCOL":[protocol_str]})
                df_seed = df_seed.append(df, ignore_index = True)

                # df = pd.DataFrame({"TYPE":["W1", "W2", "H1", "H2", "M1", "M2", "D1", "U1"], "VALUE":[wifiE/(iniE-remE), wifiE/iniE, hoverE/(iniE-remE), hoverE/iniE, moveE/(iniE-remE), moveE/iniE, remE/iniE,(iniE-remE)/iniE], "PROTOCOL": [protocol_str, protocol_str, protocol_str, protocol_str, protocol_str, protocol_str, protocol_str, protocol_str]})
                # df_custo = df_custo.append(df, ignore_index = True)
            
    # if teste:
    #     print "Final data: "+str(df_seed)
        
    # df_seed = df_seed.sort_values(['COST'])
    # if teste:
    #     print "Final sorted data: "+str(df_seed)

    df_main = df_main.append(df_seed, ignore_index = True)
    # df_seed.to_pickle("./output/"+scenario+"/"+seed+"/seed_data_battery.pkl")

df_main = df_main.sort_values(['COST'])

#### BATERIA UTIL E DESCARTADA
# bateria utilizada e descartada - POINTPLOT
fig1 = plt.figure(1)
ax1 = fig1.add_subplot(111)
ax1 = sns.pointplot(x="COST", y="U1", data=df_main, ax = ax1, join=False, capsize=.2)
ax1.set_xlabel("Custo")
ax1.set_ylabel(u"$(B_{t}^{u_i}-B_{a}^{u_i}) / B_{t}^{u_i}$")

fig2 = plt.figure(2)
ax2 = fig2.add_subplot(111)
ax2 = sns.pointplot(x="COST", y="D1", data=df_main, ax = ax2, join=False, capsize=.2)
ax2.set_xlabel("Custo")
ax2.set_ylabel(u"$B_{a}^{u_i} / B_{t}^{u_i}$")

fig1.savefig("./output/"+scenario+"/u1_pointplot.eps")
fig2.savefig("./output/"+scenario+"/d1_pointplot.eps")
fig1.clf()
fig2.clf()

# bateria utilizada e descartada - BOXPLOT
fig1 = plt.figure(1)
ax1 = fig1.add_subplot(111)
ax1 = sns.boxplot(x="COST", y="U1", data=df_main, ax = ax1)
ax1.set_xlabel("Custo")
ax1.set_ylabel(u"$(B_{t}^{u_i}-B_{a}^{u_i}) / B_{t}^{u_i}$")

fig2 = plt.figure(2)
ax2 = fig2.add_subplot(111)
ax2 = sns.boxplot(x="COST", y="D1", data=df_main, ax = ax2)
ax2.set_xlabel("Custo")
ax2.set_ylabel(u"$B_{a}^{u_i} / B_{t}^{u_i}$")

fig1.savefig("./output/"+scenario+"/u1_"+"boxplot.eps")
fig2.savefig("./output/"+scenario+"/d1_"+"boxplot.eps")
fig1.clf()
fig2.clf()

# bateria utilizada e descartada - BOXPLOT NOUT
fig1 = plt.figure(1)
ax1 = fig1.add_subplot(111)
ax1 = sns.boxplot(x="COST", y="U1", data=df_main, ax = ax1, showfliers=False)
ax1.set_xlabel("Custo")
ax1.set_ylabel(u"$(B_{t}^{u_i}-B_{a}^{u_i}) / B_{t}^{u_i}$")

fig2 = plt.figure(2)
ax2 = fig2.add_subplot(111)
ax2 = sns.boxplot(x="COST", y="D1", data=df_main, ax = ax2, showfliers=False)
ax2.set_xlabel("Custo")
ax2.set_ylabel(u"$B_{a}^{u_i} / B_{t}^{u_i}$")

fig1.savefig("./output/"+scenario+"/u1_"+"boxplot_nout.eps")
fig2.savefig("./output/"+scenario+"/d1_"+"boxplot_nout.eps")
fig1.clf()
fig2.clf()

# bateria utilizada e descartada - BOXPLOT NOUT SEEDS
fig1 = plt.figure(1)
ax1 = fig1.add_subplot(111)
ax1 = sns.boxplot(x="SEED", y="U1", data=df_main, hue="COST", ax = ax1, showfliers=False)
lgd = ax1.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=3)
ax1.set_xlabel("Custo")
ax1.set_ylabel(u"$(B_{t}^{u_i}-B_{a}^{u_i}) / B_{t}^{u_i}$")

fig2 = plt.figure(2)
ax2 = fig2.add_subplot(111)
ax2 = sns.boxplot(x="SEED", y="D1", data=df_main, hue="COST", ax = ax2, showfliers=False)
lgd = ax2.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=3)
ax2.set_xlabel("Custo")
ax2.set_ylabel(u"$B_{a}^{u_i} / B_{t}^{u_i}$")

fig1.savefig("./output/"+scenario+"/u1_seeds_"+"boxplot_nout.eps")
fig2.savefig("./output/"+scenario+"/d1_seeds_"+"boxplot_nout.eps")
fig1.clf()
fig2.clf()

#### CONSUMO POR WIFI
# consumo por wifi - POINTPLOT
fig1 = plt.figure(1)
ax1 = fig1.add_subplot(111)
ax1 = sns.pointplot(x="COST", y="W1", data=df_main, ax = ax1, join=False, capsize=.2)
ax1.set_xlabel("Custo")
ax1.set_ylabel(u"$ECw_{u_i} / (B_{t}^{u_i}-B_{a}^{u_i})$")

fig2 = plt.figure(2)
ax2 = fig2.add_subplot(111)
ax2 = sns.pointplot(x="COST", y="W2", data=df_main, ax = ax2, join=False, capsize=.2)
ax2.set_xlabel("Custo")
ax2.set_ylabel(u"$ECw_{u_i} / B_{t}^{u_i}$")

fig1.savefig("./output/"+scenario+"/w1_"+"pointplot.eps")
fig2.savefig("./output/"+scenario+"/w2_"+"pointplot.eps")

# configure ylim
ax1.set_ylim(min(ax1.get_ylim()[0],ax2.get_ylim()[0]), max(ax1.get_ylim()[1], ax2.get_ylim()[1]))
ax2.set_ylim(min(ax1.get_ylim()[0],ax2.get_ylim()[0]), max(ax1.get_ylim()[1], ax2.get_ylim()[1]))

fig1.savefig("./output/"+scenario+"/w1_lim_"+"pointplot.eps")
fig2.savefig("./output/"+scenario+"/w2_lim_"+"pointplot.eps")
fig1.clf()
fig2.clf()

# consumo por wifi - POINTPLOT SEEDS
fig1 = plt.figure(1)
ax1 = fig1.add_subplot(111)
ax1 = sns.pointplot(x="SEED", y="W1", data=df_main, hue="COST", ax = ax1, join=False, capsize=.2)
lgd = ax1.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=3)
ax1.set_xlabel("Seed")
ax1.set_ylabel(u"$ECw_{u_i} / (B_{t}^{u_i}-B_{a}^{u_i})$")

fig2 = plt.figure(2)
ax2 = fig2.add_subplot(111)
ax2 = sns.pointplot(x="SEED", y="W2", data=df_main, hue="COST", ax = ax2, join=False, capsize=.2)
lgd = ax2.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=3)
ax2.set_xlabel("Seed")
ax2.set_ylabel(u"$ECw_{u_i} / B_{t}^{u_i}$")

fig1.savefig("./output/"+scenario+"/w1_seeds_"+"pointplot.eps")
fig2.savefig("./output/"+scenario+"/w2_seeds_"+"pointplot.eps")

# configure ylim
ax1.set_ylim(min(ax1.get_ylim()[0],ax2.get_ylim()[0]), max(ax1.get_ylim()[1], ax2.get_ylim()[1]))
ax2.set_ylim(min(ax1.get_ylim()[0],ax2.get_ylim()[0]), max(ax1.get_ylim()[1], ax2.get_ylim()[1]))

fig1.savefig("./output/"+scenario+"/w1_seeds_lim_"+"pointplot.eps")
fig2.savefig("./output/"+scenario+"/w2_seeds_lim_"+"pointplot.eps")
fig1.clf()
fig2.clf()

# consumo por wifi - BOXPLOT
fig1 = plt.figure(1)
ax1 = fig1.add_subplot(111)
ax1 = sns.boxplot(x="COST", y="W1", data=df_main, ax = ax1)
ax1.set_xlabel("Custo")
ax1.set_ylabel(u"$ECw_{u_i} / (B_{t}^{u_i}-B_{a}^{u_i})$")

fig2 = plt.figure(2)
ax2 = fig2.add_subplot(111)
ax2 = sns.boxplot(x="COST", y="W2", data=df_main, ax = ax2)
ax2.set_xlabel("Custo")
ax2.set_ylabel(u"$ECw_{u_i} / B_{t}^{u_i}$")

# configure ylim
ax1.set_ylim(min(ax1.get_ylim()[0],ax2.get_ylim()[0]), max(ax1.get_ylim()[1], ax2.get_ylim()[1]))
ax2.set_ylim(min(ax1.get_ylim()[0],ax2.get_ylim()[0]), max(ax1.get_ylim()[1], ax2.get_ylim()[1]))

fig1.savefig("./output/"+scenario+"/w1_"+"boxplot.eps")
fig2.savefig("./output/"+scenario+"/w2_"+"boxplot.eps")
fig1.clf()
fig2.clf()

# consumo por wifi - BOXPLOT NOUT
fig1 = plt.figure(1)
ax1 = fig1.add_subplot(111)
ax1 = sns.boxplot(x="COST", y="W1", data=df_main, ax = ax1, showfliers=False)
ax1.set_xlabel("Custo")
ax1.set_ylabel(u"$ECw_{u_i} / (B_{t}^{u_i}-B_{a}^{u_i})$")

fig2 = plt.figure(2)
ax2 = fig2.add_subplot(111)
ax2 = sns.boxplot(x="COST", y="W2", data=df_main, ax = ax2, showfliers=False)
ax2.set_xlabel("Custo")
ax2.set_ylabel(u"$ECw_{u_i} / B_{t}^{u_i}$")

# configure ylim
ax1.set_ylim(min(ax1.get_ylim()[0],ax2.get_ylim()[0]), max(ax1.get_ylim()[1], ax2.get_ylim()[1]))
ax2.set_ylim(min(ax1.get_ylim()[0],ax2.get_ylim()[0]), max(ax1.get_ylim()[1], ax2.get_ylim()[1]))

fig1.savefig("./output/"+scenario+"/w1_"+"boxplot_nout.eps")
fig2.savefig("./output/"+scenario+"/w2_"+"boxplot_nout.eps")
fig1.clf()
fig2.clf()

# consumo por wifi - BOXPLOT NOUT SEEDS
fig1 = plt.figure(1)
ax1 = fig1.add_subplot(111)
ax1 = sns.boxplot(x="SEED", y="W1", data=df_main, hue="COST", ax = ax1, showfliers=False)
lgd = ax1.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=3)
ax1.set_xlabel("Custo")
ax1.set_ylabel(u"$ECw_{u_i} / (B_{t}^{u_i}-B_{a}^{u_i})$")

fig2 = plt.figure(2)
ax2 = fig2.add_subplot(111)
ax2 = sns.boxplot(x="SEED", y="W2", data=df_main, hue="COST", ax = ax2, showfliers=False)
lgd = ax2.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=3)
ax2.set_xlabel("Custo")
ax2.set_ylabel(u"$ECw_{u_i} / B_{t}^{u_i}$")

# configure ylim
ax1.set_ylim(min(ax1.get_ylim()[0],ax2.get_ylim()[0]), max(ax1.get_ylim()[1], ax2.get_ylim()[1]))
ax2.set_ylim(min(ax1.get_ylim()[0],ax2.get_ylim()[0]), max(ax1.get_ylim()[1], ax2.get_ylim()[1]))

fig1.savefig("./output/"+scenario+"/w1_seeds_"+"boxplot_nout.eps")
fig2.savefig("./output/"+scenario+"/w2_seeds_"+"boxplot_nout.eps")
fig1.clf()
fig2.clf()

##### CONSUMO POR MOVIMENTO
# consumo por movimento - POINTPLOT
fig1 = plt.figure(1)
ax1 = fig1.add_subplot(111)
ax1 = sns.pointplot(x="COST", y="M1", data=df_main, ax = ax1, join=False, capsize=.2)
ax1.set_xlabel("Custo")
ax1.set_ylabel(u"$ECm_{u_i} / (B_{t}^{u_i}-B_{a}^{u_i})$")

fig2 = plt.figure(2)
ax2 = fig2.add_subplot(111)
ax2 = sns.pointplot(x="COST", y="M2", data=df_main, ax = ax2, join=False, capsize=.2)
ax2.set_xlabel("Custo")
ax2.set_ylabel(u"$ECm_{u_i} / B_{t}^{u_i}$")

fig1.savefig("./output/"+scenario+"/m1_"+"pointplot.eps")
fig2.savefig("./output/"+scenario+"/m2_"+"pointplot.eps")

# configure ylim
ax1.set_ylim(min(ax1.get_ylim()[0],ax2.get_ylim()[0]), max(ax1.get_ylim()[1], ax2.get_ylim()[1]))
ax2.set_ylim(min(ax1.get_ylim()[0],ax2.get_ylim()[0]), max(ax1.get_ylim()[1], ax2.get_ylim()[1]))

fig1.savefig("./output/"+scenario+"/m1_lim_"+"pointplot.eps")
fig2.savefig("./output/"+scenario+"/m2_lim_"+"pointplot.eps")
fig1.clf()
fig2.clf()

# consumo por movimento - POINTPLOT SEEDS
fig1 = plt.figure(1)
ax1 = fig1.add_subplot(111)
ax1 = sns.pointplot(x="SEED", y="M1", data=df_main, hue="COST", ax = ax1, join=False, capsize=.2)
lgd = ax1.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=3)
ax1.set_xlabel("Seed")
ax1.set_ylabel(u"$ECm_{u_i} / (B_{t}^{u_i}-B_{a}^{u_i})$")

fig2 = plt.figure(2)
ax2 = fig2.add_subplot(111)
ax2 = sns.pointplot(x="SEED", y="M2", data=df_main, hue="COST", ax = ax2, join=False, capsize=.2)
lgd = ax2.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=3)
ax2.set_xlabel("Seed")
ax2.set_ylabel(u"$ECm_{u_i} / B_{t}^{u_i}$")

fig1.savefig("./output/"+scenario+"/m1_seeds_"+"pointplot.eps")
fig2.savefig("./output/"+scenario+"/m2_seeds_"+"pointplot.eps")

# configure ylim
ax1.set_ylim(min(ax1.get_ylim()[0],ax2.get_ylim()[0]), max(ax1.get_ylim()[1], ax2.get_ylim()[1]))
ax2.set_ylim(min(ax1.get_ylim()[0],ax2.get_ylim()[0]), max(ax1.get_ylim()[1], ax2.get_ylim()[1]))

fig1.savefig("./output/"+scenario+"/m1_seeds_lim_"+"pointplot.eps")
fig2.savefig("./output/"+scenario+"/m2_seeds_lim_"+"pointplot.eps")
fig1.clf()
fig2.clf()

# consumo por movimento - BOXPLOT
fig1 = plt.figure(1)
ax1 = fig1.add_subplot(111)
ax1 = sns.boxplot(x="COST", y="M1", data=df_main, ax = ax1)
ax1.set_xlabel("Custo")
ax1.set_ylabel(u"$ECm_{u_i} / (B_{t}^{u_i}-B_{a}^{u_i})$")

fig2 = plt.figure(2)
ax2 = fig2.add_subplot(111)
ax2 = sns.boxplot(x="COST", y="M2", data=df_main, ax = ax2)
ax2.set_xlabel("Custo")
ax2.set_ylabel(u"$ECm_{u_i} / B_{t}^{u_i}$")

# configure ylim
ax1.set_ylim(min(ax1.get_ylim()[0],ax2.get_ylim()[0]), max(ax1.get_ylim()[1], ax2.get_ylim()[1]))
ax2.set_ylim(min(ax1.get_ylim()[0],ax2.get_ylim()[0]), max(ax1.get_ylim()[1], ax2.get_ylim()[1]))

fig1.savefig("./output/"+scenario+"/m1_"+"boxplot.eps")
fig2.savefig("./output/"+scenario+"/m2_"+"boxplot.eps")
fig1.clf()
fig2.clf()

# consumo por movimento - BOXPLOT NOUT
fig1 = plt.figure(1)
ax1 = fig1.add_subplot(111)
ax1 = sns.boxplot(x="COST", y="M1", data=df_main, ax = ax1, showfliers=False)
ax1.set_xlabel("Custo")
ax1.set_ylabel(u"$ECm_{u_i} / (B_{t}^{u_i}-B_{a}^{u_i})$")

fig2 = plt.figure(2)
ax2 = fig2.add_subplot(111)
ax2 = sns.boxplot(x="COST", y="M2", data=df_main, ax = ax2, showfliers=False)
ax2.set_xlabel("Custo")
ax2.set_ylabel(u"$ECm_{u_i} / B_{t}^{u_i}$")

# configure ylim
ax1.set_ylim(min(ax1.get_ylim()[0],ax2.get_ylim()[0]), max(ax1.get_ylim()[1], ax2.get_ylim()[1]))
ax2.set_ylim(min(ax1.get_ylim()[0],ax2.get_ylim()[0]), max(ax1.get_ylim()[1], ax2.get_ylim()[1]))

fig1.savefig("./output/"+scenario+"/m1_"+"boxplot_nout.eps")
fig2.savefig("./output/"+scenario+"/m2_"+"boxplot_nout.eps")
fig1.clf()
fig2.clf()

# consumo por movimento - BOXPLOT NOUT SEEDS
fig1 = plt.figure(1)
ax1 = fig1.add_subplot(111)
ax1 = sns.boxplot(x="SEED", y="M1", data=df_main, hue="COST", ax = ax1, showfliers=False)
lgd = ax1.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=3)
ax1.set_xlabel("Custo")
ax1.set_ylabel(u"$ECm_{u_i} / (B_{t}^{u_i}-B_{a}^{u_i})$")

fig2 = plt.figure(2)
ax2 = fig2.add_subplot(111)
ax2 = sns.boxplot(x="SEED", y="M2", data=df_main, hue="COST", ax = ax2, showfliers=False)
lgd = ax2.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=3)
ax2.set_xlabel("Custo")
ax2.set_ylabel(u"$ECm_{u_i} / B_{t}^{u_i}$")

# configure ylim
ax1.set_ylim(min(ax1.get_ylim()[0],ax2.get_ylim()[0]), max(ax1.get_ylim()[1], ax2.get_ylim()[1]))
ax2.set_ylim(min(ax1.get_ylim()[0],ax2.get_ylim()[0]), max(ax1.get_ylim()[1], ax2.get_ylim()[1]))

fig1.savefig("./output/"+scenario+"/m1_seeds_"+"boxplot_nout.eps")
fig2.savefig("./output/"+scenario+"/m2_seeds_"+"boxplot_nout.eps")
fig1.clf()
fig2.clf()


##### CONSUMO POR FLUTUAR
# consumo por flutuar - POINTPLOT
fig1 = plt.figure(1)
ax1 = fig1.add_subplot(111)
ax1 = sns.pointplot(x="COST", y="H1", data=df_main, ax = ax1, join=False, capsize=.2)
ax1.set_xlabel("Custo")
ax1.set_ylabel(u"$ECh_{u_i} / (B_{t}^{u_i}-B_{a}^{u_i})$")

fig2 = plt.figure(2)
ax2 = fig2.add_subplot(111)
ax2 = sns.pointplot(x="COST", y="H2", data=df_main, ax = ax2, join=False, capsize=.2)
ax2.set_xlabel("Custo")
ax2.set_ylabel(u"$ECh_{u_i} / B_{t}^{u_i}$")

fig1.savefig("./output/"+scenario+"/h1_"+"pointplot.eps")
fig2.savefig("./output/"+scenario+"/h2_"+"pointplot.eps")

# configure ylim
ax1.set_ylim(min(ax1.get_ylim()[0],ax2.get_ylim()[0]), max(ax1.get_ylim()[1], ax2.get_ylim()[1]))
ax2.set_ylim(min(ax1.get_ylim()[0],ax2.get_ylim()[0]), max(ax1.get_ylim()[1], ax2.get_ylim()[1]))

fig1.savefig("./output/"+scenario+"/h1_lim_"+"pointplot.eps")
fig2.savefig("./output/"+scenario+"/h2_lim_"+"pointplot.eps")
fig1.clf()
fig2.clf()

# consumo por flutuar - POINTPLOT SEEDS
fig1 = plt.figure(1)
ax1 = fig1.add_subplot(111)
ax1 = sns.pointplot(x="SEED", y="H1", data=df_main, hue="COST", ax = ax1, join=False, capsize=.2)
lgd = ax1.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=3)
ax1.set_xlabel("Seed")
ax1.set_ylabel(u"$ECh_{u_i} / (B_{t}^{u_i}-B_{a}^{u_i})$")

fig2 = plt.figure(2)
ax2 = fig2.add_subplot(111)
ax2 = sns.pointplot(x="SEED", y="H2", data=df_main, hue="COST", ax = ax2, join=False, capsize=.2)
lgd = ax2.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=3)
ax2.set_xlabel("Seed")
ax2.set_ylabel(u"$ECh_{u_i} / B_{t}^{u_i}$")

fig1.savefig("./output/"+scenario+"/h1_seeds_"+"pointplot.eps")
fig2.savefig("./output/"+scenario+"/h2_seeds_"+"pointplot.eps")

# configure ylim
ax1.set_ylim(min(ax1.get_ylim()[0],ax2.get_ylim()[0]), max(ax1.get_ylim()[1], ax2.get_ylim()[1]))
ax2.set_ylim(min(ax1.get_ylim()[0],ax2.get_ylim()[0]), max(ax1.get_ylim()[1], ax2.get_ylim()[1]))

fig1.savefig("./output/"+scenario+"/h1_seeds_lim_"+"pointplot.eps")
fig2.savefig("./output/"+scenario+"/h2_seeds_lim_"+"pointplot.eps")
fig1.clf()
fig2.clf()

# consumo por flutuar - BOXPLOT
fig1 = plt.figure(1)
ax1 = fig1.add_subplot(111)
ax1 = sns.boxplot(x="COST", y="H1", data=df_main, ax = ax1)
ax1.set_xlabel("Custo")
ax1.set_ylabel(u"$ECh_{u_i} / (B_{t}^{u_i}-B_{a}^{u_i})$")

fig2 = plt.figure(2)
ax2 = fig2.add_subplot(111)
ax2 = sns.boxplot(x="COST", y="H2", data=df_main, ax = ax2)
ax2.set_xlabel("Custo")
ax2.set_ylabel(u"$ECh_{u_i} / B_{t}^{u_i}$")

# configure ylim
ax1.set_ylim(min(ax1.get_ylim()[0],ax2.get_ylim()[0]), max(ax1.get_ylim()[1], ax2.get_ylim()[1]))
ax2.set_ylim(min(ax1.get_ylim()[0],ax2.get_ylim()[0]), max(ax1.get_ylim()[1], ax2.get_ylim()[1]))

fig1.savefig("./output/"+scenario+"/h1_"+"boxplot.eps")
fig2.savefig("./output/"+scenario+"/h2_"+"boxplot.eps")
fig1.clf()
fig2.clf()

# consumo por flutuar - BOXPLOT NOUT
fig1 = plt.figure(1)
ax1 = fig1.add_subplot(111)
ax1 = sns.boxplot(x="COST", y="H1", data=df_main, ax = ax1, showfliers=False)
ax1.set_xlabel("Custo")
ax1.set_ylabel(u"$ECh_{u_i} / (B_{t}^{u_i}-B_{a}^{u_i})$")

fig2 = plt.figure(2)
ax2 = fig2.add_subplot(111)
ax2 = sns.boxplot(x="COST", y="H2", data=df_main, ax = ax2, showfliers=False)
ax2.set_xlabel("Custo")
ax2.set_ylabel(u"$ECh_{u_i} / B_{t}^{u_i}$")

# configure ylim
ax1.set_ylim(min(ax1.get_ylim()[0],ax2.get_ylim()[0]), max(ax1.get_ylim()[1], ax2.get_ylim()[1]))
ax2.set_ylim(min(ax1.get_ylim()[0],ax2.get_ylim()[0]), max(ax1.get_ylim()[1], ax2.get_ylim()[1]))

fig1.savefig("./output/"+scenario+"/h1_"+"boxplot_nout.eps")
fig2.savefig("./output/"+scenario+"/h2_"+"boxplot_nout.eps")
fig1.clf()
fig2.clf()

# consumo por flutuar - BOXPLOT NOUT SEEDS
fig1 = plt.figure(1)
ax1 = fig1.add_subplot(111)
ax1 = sns.boxplot(x="SEED", y="H1", data=df_main, hue="COST", ax = ax1, showfliers=False)
lgd = ax1.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=3)
ax1.set_xlabel("Custo")
ax1.set_ylabel(u"$ECh_{u_i} / (B_{t}^{u_i}-B_{a}^{u_i})$")

fig2 = plt.figure(2)
ax2 = fig2.add_subplot(111)
ax2 = sns.boxplot(x="SEED", y="H2", data=df_main, hue="COST", ax = ax2, showfliers=False)
lgd = ax2.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=3)
ax2.set_xlabel("Custo")
ax2.set_ylabel(u"$ECh_{u_i} / B_{t}^{u_i}$")

# configure ylim
ax1.set_ylim(min(ax1.get_ylim()[0],ax2.get_ylim()[0]), max(ax1.get_ylim()[1], ax2.get_ylim()[1]))
ax2.set_ylim(min(ax1.get_ylim()[0],ax2.get_ylim()[0]), max(ax1.get_ylim()[1], ax2.get_ylim()[1]))

fig1.savefig("./output/"+scenario+"/h1_seeds_"+"boxplot_nout.eps")
fig2.savefig("./output/"+scenario+"/h2_seeds_"+"boxplot_nout.eps")
fig1.clf()
fig2.clf()

df_main.to_pickle("./output/"+scenario+"/main_data_battery.pkl")