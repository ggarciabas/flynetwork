# -*- coding: UTF-8 -*-
# libraries and data
import glob
import os
import sys
import numpy as np
import matplotlib.pyplot as plt


teste = True
if sys.argv[1] == "False":
    teste = False
scenario = sys.argv[2]
main_path = "./output/"+scenario+"/"

for custo_name in glob.glob(main_path+'custo_*/'):
    custo = os.path.dirname(custo_name).split('/')[-1]
    if teste:
        print "Custo: "+custo
    cli_files = glob.glob(main_path+custo+'/dhcp/client_*')
    data_ASSOC = []
    data_DEASSOC = []
    data_NEWLEASE = []
    data_EXPIRYLEASE = []

    i = 0
    for path in cli_files:
        file_c = open(path, 'r')
        for line in file_c:
            data = [x for x in line.split(' ')]
            if data[1] == "ASSOC":
                data_ASSOC.append([float(data[0]), i])
            elif data[1] == "DEASSOC":
                data_DEASSOC.append([float(data[0]), i])
            elif data[1] == "NEWLEASE":
                data_NEWLEASE.append([float(data[0]), i])
            elif data[1] == "EXPIRYLEASE":
                data_EXPIRYLEASE.append([float(data[0]), i])
        i = i + 1

    print data_DEASSOC
    if len(data_DEASSOC) > 0:
        data_DEASSOC = np.array(data_DEASSOC)
        x,y = data_DEASSOC.T
        plt.scatter(x,y, s=1, c='k', label=u"Desconectado")

    print data_EXPIRYLEASE
    if len(data_EXPIRYLEASE) > 0:
        data_NEWLEASE = np.array(data_EXPIRYLEASE)
        x,y = data_EXPIRYLEASE.T
        plt.scatter(x,y, s=1, c='g', label="Expirou IP")

    print data_NEWLEASE
    if len(data_NEWLEASE) > 0:
        data_NEWLEASE = np.array(data_NEWLEASE)
        x,y = data_NEWLEASE.T
        plt.scatter(x,y, s=1, c='b', label="Novo IP")

    print data_ASSOC
    if len(data_ASSOC) > 0:
        data_ASSOC = np.array(data_ASSOC)
        x,y = data_ASSOC.T
        plt.scatter(x,y, s=1, c='r', label="Conectado")


    plt.title (u"DHCP para clientes")
    plt.xlabel(u"Tempo (s)")
    plt.ylabel(u"Cliente")
    lgd = plt.legend(loc='upper center', bbox_to_anchor=(0.5, 1.16), fancybox=True, shadow=True, ncol=5)

    plt.savefig(main_path+custo+'/client_dhcp.svg', bbox_extra_artists=(lgd,), bbox_inches='tight')
    plt.savefig(main_path+custo+'/client_dhcp.eps', bbox_extra_artists=(lgd,), bbox_inches='tight')
    plt.savefig(main_path+custo+'/client_dhcp.png', bbox_extra_artists=(lgd,), bbox_inches='tight')
    # plt.show()
    plt.close()
