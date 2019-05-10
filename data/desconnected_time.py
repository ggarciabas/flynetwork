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

            for client in glob.glob(main_path+seed+'/'+protocol+'/'+custo+'/dhcp/client_??.txt'):
                client_f = os.path.basename(client)
                if teste:
                    print client_f

                try:
                    file = open(client,'r')
                except IOError:
                    exit()

                last_time = 0.0 # ultimo tempo 
                conn_time = 0.0 # tempo conectado
                desc_time = 0.0 # tempo desconectado
                for line in file:
                    line_data = [x for x in line.split(' ')]
                    if teste:
                        print line_data
                    if line_data[1] == "ASSOC":
                        print "Conectado"
                        # se houve conexao é pq estava desconectado
                        diff = float(line_data[0])-last_time
                        if teste:
                            print "Diff: "+str(diff)
                        desc_time = desc_time + diff
                        last_time = float(line_data[0])
                    elif line_data[1] == "DEASSOC":
                        print "Desconectado"
                        # se houve desconexao é pq estava conectado
                        diff = float(line_data[0])-last_time
                        if teste:
                            print "Diff: "+str(diff)
                        conn_time = conn_time + diff
                        last_time = float(line_data[0])

                if teste:
                    print "Client: " + client + " contime: "+str(conn_time) + " desctime: "+str(desc_time)

                df = pd.DataFrame({"SEED":[seed], "COST":[custo], "CT":[conn_time/s_time*100], "DT":[desc_time/s_time*100], "TAX":[desc_time/conn_time if conn_time > 0 else 1], "CLIENT":[client_f], "PROTOCOL":[protocol_str]})
                df_seed = df_seed.append(df, ignore_index = True)

                file.close()   

    if teste:
        print "Final data: "+str(df_seed)
        
    df_seed = df_seed.sort_values(['COST'])
    if teste:
        print "Final sorted data: "+str(df_seed)

    ax = sns.boxplot(x="COST", y="DT", data=df_seed, hue="PROTOCOL")
    legend = ax.legend()
    legend.texts[0].set_text('Protocol')
    lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=2)
    plt.xlabel("Custo")
    plt.ylabel(u"Tempo desconectado (%)")
    # plt.show()
    # plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
    # plt.savefig("./output/"+scenario+"/"+seed+"/desc_time.svg")
    plt.savefig("./output/"+scenario+"/"+seed+"/desc_time.eps")
    # plt.savefig("./output/"+scenario+"/"+seed+"/desc_time.png")
    plt.close()

    ax = sns.boxplot(x="COST", y="TAX", data=df_seed, hue="PROTOCOL")
    legend = ax.legend()
    legend.texts[0].set_text('Protocol')
    lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=2)
    plt.xlabel("Custo")
    plt.ylabel(u"Desc.Time/Con.Time")
    # plt.show()
    # plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
    # plt.savefig("./output/"+scenario+"/"+seed+"/tax_time.svg")
    plt.savefig("./output/"+scenario+"/"+seed+"/tax_time.eps")
    # plt.savefig("./output/"+scenario+"/"+seed+"/tax_time.png")
    plt.close()

    ax = sns.boxplot(x="COST", y="CT", data=df_seed, hue="PROTOCOL")
    legend = ax.legend()
    legend.texts[0].set_text('Protocol')
    lgd = ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), fancybox=True, shadow=True, ncol=2)
    plt.xlabel("Custo")
    plt.ylabel(u"Tempo conectado (%)")
    # plt.show()
    # plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
    # plt.savefig("./output/"+scenario+"/"+seed+"/conn_time.svg")
    plt.savefig("./output/"+scenario+"/"+seed+"/conn_time.eps")
    # plt.savefig("./output/"+scenario+"/"+seed+"/conn_time.png")
    plt.close()
    df_seed.to_pickle("./output/"+scenario+"/"+seed+"/seed_data.pkl")

    df_main = df_main.append(df_seed, ignore_index = True)

df_main = df_main.sort_values(['COST'])

ax = sns.boxplot(x="COST", y="DT", data=df_main)
plt.xlabel("Custo")
plt.ylabel(u"Tempo desconectado (%)")
# plt.show()
# plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
# plt.savefig("./output/"+scenario+"/desc_time.svg")
plt.savefig("./output/"+scenario+"/desc_time.eps")
# plt.savefig("./output/"+scenario+"/desc_time.png")
plt.close()

ax = sns.boxplot(x="COST", y="DT", data=df_main, showfliers=False)
plt.xlabel("Custo")
plt.ylabel(u"Tempo desconectado (%)")
# plt.show()
# plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
# plt.savefig("./output/"+scenario+"/desc_time_nout.svg")
plt.savefig("./output/"+scenario+"/desc_time_nout.eps")
# plt.savefig("./output/"+scenario+"/desc_time_nout.png")
plt.close()

ax = sns.boxplot(x="COST", y="TAX", data=df_main)
plt.xlabel("Custo")
plt.ylabel(u"Desc.Time/Con.Time")
# plt.show()
# plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
# plt.savefig("./output/"+scenario+"/tax_time.svg")
plt.savefig("./output/"+scenario+"/tax_time.eps")
# plt.savefig("./output/"+scenario+"/tax_time.png")
plt.close()

ax = sns.boxplot(x="COST", y="TAX", data=df_main, showfliers=False)
plt.xlabel("Custo")
plt.ylabel(u"Desc.Time/Con.Time")
# plt.show()
# plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
# plt.savefig("./output/"+scenario+"/tax_time.svg")
plt.savefig("./output/"+scenario+"/tax_time_nout.eps")
# plt.savefig("./output/"+scenario+"/tax_time.png")
plt.close()

ax = sns.boxplot(x="COST", y="CT", data=df_main)
plt.xlabel("Custo")
plt.ylabel(u"Tempo conectado (%)")
# plt.show()
# plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
# plt.savefig("./output/"+scenario+"/conn_time.svg")
plt.savefig("./output/"+scenario+"/conn_time.eps")
# plt.savefig("./output/"+scenario+"/conn_time.png") 
plt.close() 

ax = sns.boxplot(x="COST", y="CT", data=df_main, showfliers=False)
plt.xlabel("Custo")
plt.ylabel(u"Tempo conectado (%)")
# plt.show()
# plt.title("Comparação tempo de", fontsize=13, fontweight=0, color='black', style='italic')
# plt.savefig("./output/"+scenario+"/conn_time.svg")
plt.savefig("./output/"+scenario+"/conn_time_nout.eps")
# plt.savefig("./output/"+scenario+"/conn_time.png") 
plt.close() 

df.to_pickle("./output/"+scenario+"/main_data.pkl")