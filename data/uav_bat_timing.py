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
    df_seed_timing = pd.DataFrame()
    for protocol_name in glob.glob(main_path+seed+'/*/'):
        protocol = os.path.dirname(protocol_name).split('/')[-1]
        if int(protocol) == 1:
            protocol_str = str("OLSR")
        elif int(protocol) == 2:
            protocol_str = str("AODV")
        elif int(protocol) == 3:
            protocol_str = str("DSDV")

        df_custo = pd.DataFrame()
        for custo_name in glob.glob(main_path+seed+'/'+protocol+'/custo_*/'):
            custo = os.path.dirname(custo_name).split('/')[-1]
            if teste:
                print "Custo: "+custo

            # uav timming
            # df_timing = pd.DataFrame()
            for uav_timing in glob.glob(main_path+seed+'/'+protocol+'/'+custo+'/uav_energy/uav_timing_energy_*.txt'):
                # TIME NODE_ID INI_E REM_E WIFI_W CLI_E MOVE_E HOVER_E DEPLETED
                rem_ant = -1
                if teste:
                    print uav_timing
                for line in open(uav_timing, "r"):
                    line_data = [x for x in line.split(' ')]
                    time = float(line_data[0])
                    iniE = float(line_data[2])   
                    remE = float(line_data[3])  
                    if rem_ant == -1:
                        rem_ant = iniE
                    moveE = float(line_data[6])
                    hoverE = float(line_data[7])
                    wifiE = rem_ant - remE - moveE - hoverE    # TODO modificar para leitura direta do 5 valor!     
                    rem_ant = remE

                    df = pd.DataFrame({"TIME":[time], "SEED":[seed], "COST":[custo], "W1":[wifiE/(iniE-remE)], "W2":[wifiE/iniE], "M1":[moveE/(iniE-remE)], "M2":[moveE/iniE], "H1":[hoverE/(iniE-remE)], "H2":[hoverE/iniE]})
                    df_seed_timing = df_seed_timing.append(df, ignore_index = True)  
                    df_custo = df_custo.append(df, ignore_index = True)  


        ax = sns.lineplot(x="TIME", y="W1", data=df_custo, hue="COST")
        lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=2)
        plt.xlabel("Tempo (s)")
        plt.ylabel(u"$ECw_{u_i} / (B_{t}^{u_i}-B_{a}^{u_i})$")
        # plt.show()
        # #plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
        # plt.savefig("./output/"+scenario+"/"+seed+"/w1_timing.svg")
        plt.savefig("./output/"+scenario+"/"+seed+"/"+protocol+"/"+custo+"/w1_line_timing.eps")
        # plt.savefig("./output/"+scenario+"/"+seed+"/w1_timing.png")
        plt.close()
        
    # if teste:
    #     print "Final data: "+str(df_seed_timing)

    # df_seed_timing = df_seed_timing.sort_values(["TIME"])
    # if teste:
    #     print "Final sorted data: "+str(df_seed_timing)

    # ax = sns.boxplot(x="TIME", y="W1", data=df_seed_timing, hue="COST")
    # lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=2)
    # plt.xlabel("Tempo (s)")
    # plt.ylabel(u"$ECw_{u_i} / (B_{t}^{u_i}-B_{a}^{u_i})$")
    # # plt.show()
    # # #plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
    # # plt.savefig("./output/"+scenario+"/"+seed+"/w1_timing.svg")
    # plt.savefig("./output/"+scenario+"/"+seed+"/w1_timing.eps")
    # # plt.savefig("./output/"+scenario+"/"+seed+"/w1_timing.png")
    # plt.close()

    # ax = sns.boxplot(x="TIME", y="W2", data=df_seed_timing, hue="COST")
    # lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=2)
    # plt.xlabel("Tempo (s)")
    # plt.ylabel(u"$ECw_{u_i} / B_{t}^{u_i}$")
    # # plt.show()
    # # #plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
    # # plt.savefig("./output/"+scenario+"/"+seed+"/w2_timing.svg")
    # plt.savefig("./output/"+scenario+"/"+seed+"/w2_timing.eps")
    # # plt.savefig("./output/"+scenario+"/"+seed+"/w2_timing.png")
    # plt.close()

    # ax = sns.boxplot(x="TIME", y="M1", data=df_seed_timing, hue="COST")
    # lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=2)
    # plt.xlabel("Tempo (s)")
    # plt.ylabel(u"$ECm_{u_i} / (B_{t}^{u_i}-B_{a}^{u_i})$")
    # # plt.show()
    # # #plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
    # # plt.savefig("./output/"+scenario+"/"+seed+"/m1_timing.svg")
    # plt.savefig("./output/"+scenario+"/"+seed+"/m1_timing.eps")
    # # plt.savefig("./output/"+scenario+"/"+seed+"/m1_timing.png")
    # plt.close()

    # ax = sns.boxplot(x="TIME", y="M2", data=df_seed_timing, hue="COST")
    # lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=2)
    # plt.xlabel("Tempo (s)")
    # plt.ylabel(u"$ECm_{u_i} / B_{t}^{u_i}$")
    # # plt.show()
    # # #plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
    # # plt.savefig("./output/"+scenario+"/"+seed+"/m1_timing.svg")
    # plt.savefig("./output/"+scenario+"/"+seed+"/m2_timing.eps")
    # # plt.savefig("./output/"+scenario+"/"+seed+"/m1_timing.png")
    # plt.close()

    # ax = sns.boxplot(x="TIME", y="H1", data=df_seed_timing, hue="COST")
    # lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=2)
    # plt.xlabel("Tempo (s)")
    # plt.ylabel(u"$ECh_{u_i} / (B_{t}^{u_i}-B_{a}^{u_i})$")
    # # plt.show()
    # # #plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
    # # plt.savefig("./output/"+scenario+"/"+seed+"/h1_timing.svg")
    # plt.savefig("./output/"+scenario+"/"+seed+"/h1_timing.eps")
    # # plt.savefig("./output/"+scenario+"/"+seed+"/h1_timing.png")
    # plt.close()

    # ax = sns.boxplot(x="TIME", y="H2", data=df_seed_timing, hue="COST")
    # lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=2)
    # plt.xlabel("Tempo (s)")
    # plt.ylabel(u"$ECh_{u_i} / B_{t}^{u_i}$")
    # # plt.show()
    # # #plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
    # # plt.savefig("./output/"+scenario+"/"+seed+"/h2_timing.svg")
    # plt.savefig("./output/"+scenario+"/"+seed+"/h2_timing.eps")
    # # plt.savefig("./output/"+scenario+"/"+seed+"/h2_timing.png")
    # plt.close()

    # df_main = df_main.append(df_seed, ignore_index = True)
    # df_seed.to_pickle("./output/"+scenario+"/"+seed+"/seed_data_battery.pkl")
    # legend.texts[0].set_text('Protocol')
    # lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=2)


    ax = sns.lineplot(x="TIME", y="W1", data=df_seed_timing, hue="COST")
    lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=2)
    plt.xlabel("Tempo (s)")
    plt.ylabel(u"$ECw_{u_i} / (B_{t}^{u_i}-B_{a}^{u_i})$")
    # plt.show()
    # #plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
    # plt.savefig("./output/"+scenario+"/"+seed+"/w1_timing.svg")
    plt.savefig("./output/"+scenario+"/"+seed+"/w1_line_timing.eps")
    # plt.savefig("./output/"+scenario+"/"+seed+"/w1_timing.png")
    plt.close()

    df_main = df_main.append(df_seed_timing, ignore_index = True)

# ax = sns.boxplot(x="TIME", y="W1", data=df_main, hue="COST")
# lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=2)
# plt.xlabel("Tempo (s)")
# plt.ylabel(u"$ECw_{u_i} / (B_{t}^{u_i}-B_{a}^{u_i})$")
# # plt.show()
# # #plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
# # plt.savefig("./output/"+scenario+"/w1_timing.svg")
# plt.savefig("./output/"+scenario+"/w1_timing.eps")
# # plt.savefig("./output/"+scenario+"/w1_timing.png")
# plt.close()

# ax = sns.lineplot(x="TIME", y="W1", data=df_main, hue="COST")
# lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=2)
# plt.xlabel("Tempo (s)")
# plt.ylabel(u"$ECw_{u_i} / (B_{t}^{u_i}-B_{a}^{u_i})$")
# # plt.show()
# # #plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
# # plt.savefig("./output/"+scenario+"/w1_timing.svg")
# plt.savefig("./output/"+scenario+"/w1_line_timing.eps")
# # plt.savefig("./output/"+scenario+"/w1_timing.png")
# plt.close()

# ax = sns.boxplot(x="TIME", y="W2", data=df_main, hue="COST")
# lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=2)
# plt.xlabel("Tempo (s)")
# plt.ylabel(u"$ECw_{u_i} / B_{t}^{u_i}$")
# # plt.show()
# # #plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
# # plt.savefig("./output/"+scenario+"/w2_timing.svg")
# plt.savefig("./output/"+scenario+"/w2_timing.eps")
# # plt.savefig("./output/"+scenario+"/w2_timing.png")
# plt.close()

# ax = sns.boxplot(x="TIME", y="M1", data=df_main, hue="COST")
# lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=2)
# plt.xlabel("Tempo (s)")
# plt.ylabel(u"$ECm_{u_i} / (B_{t}^{u_i}-B_{a}^{u_i})$")
# # plt.show()
# # #plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
# # plt.savefig("./output/"+scenario+"/m1_timing.svg")
# plt.savefig("./output/"+scenario+"/m1_timing.eps")
# # plt.savefig("./output/"+scenario+"/m1_timing.png")
# plt.close()

# ax = sns.boxplot(x="TIME", y="M2", data=df_main, hue="COST")
# lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=2)
# plt.xlabel("Tempo (s)")
# plt.ylabel(u"$ECm_{u_i} / B_{t}^{u_i}$")
# # plt.show()
# # #plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
# # plt.savefig("./output/"+scenario+"/m1_timing.svg")
# plt.savefig("./output/"+scenario+"/m2_timing.eps")
# # plt.savefig("./output/"+scenario+"/m1_timing.png")
# plt.close()

# ax = sns.boxplot(x="TIME", y="H1", data=df_main, hue="COST")
# lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=2)
# plt.xlabel("Tempo (s)")
# plt.ylabel(u"$ECh_{u_i} / (B_{t}^{u_i}-B_{a}^{u_i})$")
# # plt.show()
# # #plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
# # plt.savefig("./output/"+scenario+"/h1_timing.svg")
# plt.savefig("./output/"+scenario+"/h1_timing.eps")
# # plt.savefig("./output/"+scenario+"/h1_timing.png")
# plt.close()

# ax = sns.boxplot(x="TIME", y="H2", data=df_main, hue="COST")
# lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=2)
# plt.xlabel("Tempo (s)")
# plt.ylabel(u"$ECh_{u_i} / B_{t}^{u_i}$")
# # plt.show()
# # #plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
# # plt.savefig("./output/"+scenario+"/h2_timing.svg")
# plt.savefig("./output/"+scenario+"/h2_timing.eps")
# # plt.savefig("./output/"+scenario+"/h2_timing.png")
# plt.close()

# df_seed_timing.to_pickle("./output/"+scenario+"/seed_data_battery.pkl")
# df_main.to_pickle("./output/"+scenario+"/main_data_battery.pkl")