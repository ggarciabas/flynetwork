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
            if teste:
                print "Custo: "+custo

            # # structure of log file FALHA em algumas simulacoes, o resultado final não está ocorrendo!
            # # TIME UAV_ID INITIAL_E ACTUAL_E WIFI_E CLIENT_E MOVE_E HOVER_E
            # # remove duplicated lines on log file
            # lines_seen = set() # holds lines already seen
            # for line in open(main_path+seed+'/'+protocol+'/'+custo+'/uav_energy/uav_energy.txt', "r"):
            #     if line not in lines_seen: # not a duplicate
            #         lines_seen.add(line)

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
                df = pd.DataFrame({"SEED":[seed], "COST":[custo], "W1":[wifiE/(iniE-remE)], "W2":[wifiE/iniE], "M1":[moveE/(iniE-remE)], "M2":[moveE/iniE], "H1":[hoverE/(iniE-remE)], "H2":[hoverE/iniE], "D1":[remE/iniE], "U1":[(iniE-remE)/iniE], "PROTOCOL":[protocol_str]})
                df_seed = df_seed.append(df, ignore_index = True)

                df = pd.DataFrame({"TYPE":["W1", "W2", "H1", "H2", "M1", "M2", "D1", "U1"], "VALUE":[wifiE/(iniE-remE), wifiE/iniE, hoverE/(iniE-remE), hoverE/iniE, moveE/(iniE-remE), moveE/iniE, remE/iniE,(iniE-remE)/iniE], "PROTOCOL": [protocol_str, protocol_str, protocol_str, protocol_str, protocol_str, protocol_str, protocol_str, protocol_str]})
                df_custo = df_custo.append(df, ignore_index = True)

            ax = sns.boxplot(x="TYPE", y="VALUE", data=df_custo, hue="PROTOCOL")
            legend = ax.legend()
            legend.texts[0].set_text('Protocol')
            lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=2)
            plt.xlabel("Type")
            plt.ylabel("(0-1)")
            # plt.show()
            # #plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
            plt.savefig("./output/"+scenario+"/"+seed+"/cost_"+custo+".svg")
            plt.savefig("./output/"+scenario+"/"+seed+"/cost_"+custo+".eps")
            plt.savefig("./output/"+scenario+"/"+seed+"/cost_"+custo+".png")
            plt.close()
            
    if teste:
        print "Final data: "+str(df_seed)
        
    df_seed = df_seed.sort_values(['COST', "PROTOCOL"])
    if teste:
        print "Final sorted data: "+str(df_seed)


    ax = sns.boxplot(x="COST", y="U1", data=df_seed, hue="PROTOCOL")
    legend = ax.legend()
    legend.texts[0].set_text('Protocol')
    lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=2)
    plt.xlabel("Custo")
    plt.ylabel(u"$(B_{t}^{u_i}-B_{a}^{u_i}) / B_{t}^{u_i}$")
    # plt.show()
    # #plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
    plt.savefig("./output/"+scenario+"/"+seed+"/u1.svg")
    plt.savefig("./output/"+scenario+"/"+seed+"/u1.eps")
    plt.savefig("./output/"+scenario+"/"+seed+"/u1.png")
    plt.close()

    ax = sns.boxplot(x="COST", y="D1", data=df_seed, hue="PROTOCOL")
    legend = ax.legend()
    legend.texts[0].set_text('Protocol')
    lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=2)
    plt.xlabel("Custo")
    plt.ylabel(u"$B_{a}^{u_i} / B_{t}^{u_i}$")
    # plt.show()
    # #plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
    plt.savefig("./output/"+scenario+"/"+seed+"/d1.svg")
    plt.savefig("./output/"+scenario+"/"+seed+"/d1.eps")
    plt.savefig("./output/"+scenario+"/"+seed+"/d1.png")
    plt.close()

    ax = sns.boxplot(x="COST", y="U1", data=df_seed, showfliers=False, hue="PROTOCOL")
    legend = ax.legend()
    legend.texts[0].set_text('Protocol')
    lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=2)
    plt.xlabel("Custo")
    plt.ylabel(u"$(B_{t}^{u_i}-B_{a}^{u_i}) / B_{t}^{u_i}$")
    # plt.show()
    # #plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
    plt.savefig("./output/"+scenario+"/"+seed+"/u1_nout.svg")
    plt.savefig("./output/"+scenario+"/"+seed+"/u1_nout.eps")
    plt.savefig("./output/"+scenario+"/"+seed+"/u1_nout.png")
    plt.close()

    ax = sns.boxplot(x="COST", y="D1", data=df_seed, showfliers=False, hue="PROTOCOL")
    legend = ax.legend()
    legend.texts[0].set_text('Protocol')
    lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=2)
    plt.xlabel("Custo")
    plt.ylabel(u"$B_{a}^{u_i} / B_{t}^{u_i}$")
    # plt.show()
    # #plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
    plt.savefig("./output/"+scenario+"/"+seed+"/d1_nout.svg")
    plt.savefig("./output/"+scenario+"/"+seed+"/d1_nout.eps")
    plt.savefig("./output/"+scenario+"/"+seed+"/d1_nout.png")
    plt.close()

    ax = sns.boxplot(x="COST", y="W1", data=df_seed, hue="PROTOCOL")
    legend = ax.legend()
    legend.texts[0].set_text('Protocol')
    lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=2)
    plt.xlabel("Custo")
    plt.ylabel(u"$ECw_{u_i} / (B_{t}^{u_i}-B_{a}^{u_i})$")
    # plt.show()
    # #plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
    plt.savefig("./output/"+scenario+"/"+seed+"/w1.svg")
    plt.savefig("./output/"+scenario+"/"+seed+"/w1.eps")
    plt.savefig("./output/"+scenario+"/"+seed+"/w1.png")
    plt.close()

    ax = sns.boxplot(x="COST", y="W2", data=df_seed, hue="PROTOCOL")
    legend = ax.legend()
    legend.texts[0].set_text('Protocol')
    lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=2)
    plt.xlabel("Custo")
    plt.ylabel(u"$ECw_{u_i} / B_{t}^{u_i}$")
    # plt.show()
    # #plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
    plt.savefig("./output/"+scenario+"/"+seed+"/w2.svg")
    plt.savefig("./output/"+scenario+"/"+seed+"/w2.eps")
    plt.savefig("./output/"+scenario+"/"+seed+"/w2.png")
    plt.close()

    ax = sns.boxplot(x="COST", y="M1", data=df_seed, hue="PROTOCOL")
    legend = ax.legend()
    legend.texts[0].set_text('Protocol')
    lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=2)
    plt.xlabel("Custo")
    plt.ylabel(u"$ECm_{u_i} / (B_{t}^{u_i}-B_{a}^{u_i})$")
    # plt.show()
    # #plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
    plt.savefig("./output/"+scenario+"/"+seed+"/m1.svg")
    plt.savefig("./output/"+scenario+"/"+seed+"/m1.eps")
    plt.savefig("./output/"+scenario+"/"+seed+"/m1.png")
    plt.close()

    ax = sns.boxplot(x="COST", y="M2", data=df_seed, hue="PROTOCOL")
    legend = ax.legend()
    legend.texts[0].set_text('Protocol')
    lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=2)
    plt.xlabel("Custo")
    plt.ylabel(u"$ECm_{u_i} / B_{t}^{u_i}$")
    # plt.show()
    # #plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
    plt.savefig("./output/"+scenario+"/"+seed+"/m2.svg")
    plt.savefig("./output/"+scenario+"/"+seed+"/m2.eps")
    plt.savefig("./output/"+scenario+"/"+seed+"/m2.png")
    plt.close()

    ax = sns.boxplot(x="COST", y="H1", data=df_seed, hue="PROTOCOL")
    legend = ax.legend()
    legend.texts[0].set_text('Protocol')
    lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=2)
    plt.xlabel("Custo")
    plt.ylabel(u"$ECh_{u_i} / (B_{t}^{u_i}-B_{a}^{u_i})$")
    # plt.show()
    # #plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
    plt.savefig("./output/"+scenario+"/"+seed+"/h1.svg")
    plt.savefig("./output/"+scenario+"/"+seed+"/h1.eps")
    plt.savefig("./output/"+scenario+"/"+seed+"/h1.png")
    plt.close()

    ax = sns.boxplot(x="COST", y="H2", data=df_seed, hue="PROTOCOL")
    legend = ax.legend()
    legend.texts[0].set_text('Protocol')
    lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=2)
    plt.xlabel("Custo")
    plt.ylabel(u"$ECh_{u_i} / B_{t}^{u_i}$")
    # plt.show()
    # #plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
    plt.savefig("./output/"+scenario+"/"+seed+"/h2.svg")
    plt.savefig("./output/"+scenario+"/"+seed+"/h2.eps")
    plt.savefig("./output/"+scenario+"/"+seed+"/h2.png")
    plt.close()

    df_main = df_main.append(df_seed, ignore_index = True)
    df_seed.to_pickle("./output/"+scenario+"/"+seed+"/seed_data_battery.pkl")
    legend.texts[0].set_text('Protocol')
    lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=2)


df_main = df_main.sort_values(['COST', "PROTOCOL"])

ax = sns.boxplot(x="COST", y="U1", data=df_main)
plt.xlabel("Custo")
plt.ylabel(u"$(B_{t}^{u_i}-B_{a}^{u_i}) / B_{t}^{u_i}$")
# plt.show()
# #plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
plt.savefig("./output/"+scenario+"/u1.svg")
plt.savefig("./output/"+scenario+"/u1.eps")
plt.savefig("./output/"+scenario+"/u1.png")
plt.close()

ax = sns.boxplot(x="COST", y="D1", data=df_main, hue="PROTOCOL")
legend.texts[0].set_text('Protocol')
plt.xlabel("Custo")
plt.ylabel(u"$B_{a}^{u_i} / B_{t}^{u_i}$")
# plt.show()
# #plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
plt.savefig("./output/"+scenario+"/d1.svg")
plt.savefig("./output/"+scenario+"/d1.eps")
plt.savefig("./output/"+scenario+"/d1.png")
plt.close()

ax = sns.boxplot(x="COST", y="W1", data=df_main)
plt.xlabel("Custo")
plt.ylabel(u"$ECw_{u_i} / (B_{t}^{u_i}-B_{a}^{u_i})$")
# plt.show()
# #plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
plt.savefig("./output/"+scenario+"/w1.svg")
plt.savefig("./output/"+scenario+"/w1.eps")
plt.savefig("./output/"+scenario+"/w1.png")
plt.close()

ax = sns.boxplot(x="COST", y="U1", data=df_main, showfliers=False)
plt.xlabel("Custo")
plt.ylabel(u"$(B_{t}^{u_i}-B_{a}^{u_i}) / B_{t}^{u_i}$")
# plt.show()
# #plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
plt.savefig("./output/"+scenario+"/u1_nout.svg")
plt.savefig("./output/"+scenario+"/u1_nout.eps")
plt.savefig("./output/"+scenario+"/u1_nout.png")
plt.close()

ax = sns.boxplot(x="COST", y="D1", data=df_main, showfliers=False)
plt.xlabel("Custo")
plt.ylabel(u"$B_{a}^{u_i} / B_{t}^{u_i}$")
# plt.show()
# #plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
plt.savefig("./output/"+scenario+"/d1_nout.svg")
plt.savefig("./output/"+scenario+"/d1_nout.eps")
plt.savefig("./output/"+scenario+"/d1_nout.png")
plt.close()

ax = sns.boxplot(x="COST", y="W2", data=df_main)
plt.xlabel("Custo")
plt.ylabel(u"$ECw_{u_i} / B_{t}^{u_i}$")
# plt.show()
# #plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
plt.savefig("./output/"+scenario+"/w2.svg")
plt.savefig("./output/"+scenario+"/w2.eps")
plt.savefig("./output/"+scenario+"/w2.png")
plt.close()

ax = sns.boxplot(x="COST", y="M1", data=df_main)
plt.xlabel("Custo")
plt.ylabel(u"$ECm_{u_i} / (B_{t}^{u_i}-B_{a}^{u_i})$")
# plt.show()
# #plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
plt.savefig("./output/"+scenario+"/m1.svg")
plt.savefig("./output/"+scenario+"/m1.eps")
plt.savefig("./output/"+scenario+"/m1.png")
plt.close()

ax = sns.boxplot(x="COST", y="M2", data=df_main)
plt.xlabel("Custo")
plt.ylabel(u"$ECm_{u_i} / B_{t}^{u_i}$")
# plt.show()
# #plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
plt.savefig("./output/"+scenario+"/m2.svg")
plt.savefig("./output/"+scenario+"/m2.eps")
plt.savefig("./output/"+scenario+"/m2.png")
plt.close()

ax = sns.boxplot(x="COST", y="H1", data=df_main)
plt.xlabel("Custo")
plt.ylabel(u"$ECh_{u_i} / (B_{t}^{u_i}-B_{a}^{u_i})$")
# plt.show()
# #plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
plt.savefig("./output/"+scenario+"/h1.svg")
plt.savefig("./output/"+scenario+"/h1.eps")
plt.savefig("./output/"+scenario+"/h1.png")
plt.close()

ax = sns.boxplot(x="COST", y="H2", data=df_main)
plt.xlabel("Custo")
plt.ylabel(u"$ECh_{u_i} / B_{t}^{u_i}$")
# plt.show()
# #plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
plt.savefig("./output/"+scenario+"/h2.svg")
plt.savefig("./output/"+scenario+"/h2.eps")
plt.savefig("./output/"+scenario+"/h2.png")
plt.close()
df_main.to_pickle("./output/"+scenario+"/main_data_battery.pkl")