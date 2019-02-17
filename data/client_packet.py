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
    cli_files = glob.glob(main_path+custo+'/client/*')
    data_ENVIADO = []
    data_FALHA = []
    data_NAOCONECTADO = []

    i = 0
    for path in cli_files:
        file_c = open(path, 'r')
        for line in file_c:
            data = [x for x in line.split(' ')]
            if data[1] == "ENVIADO\n":
                data_ENVIADO.append([float(data[0]), i])
            elif data[1] == "NAO_CONECTADO\n":
                data_NAOCONECTADO.append([float(data[0]), i])
            elif data[1] == "FALHA\n":
                data_FALHA.append([float(data[0]), i])
        i = i + 1

    print data_NAOCONECTADO
    if len(data_NAOCONECTADO) > 0:
        data_NAOCONECTADO = np.array(data_NAOCONECTADO)
        x,y = data_NAOCONECTADO.T
        plt.scatter(x,y, s=1, c='k', label=u"Não conectado")

    print data_FALHA
    if len(data_FALHA) > 0:
        data_FALHA = np.array(data_FALHA)
        x,y = data_FALHA.T
        plt.scatter(x,y, s=1, c='r', label="Falha")

    print data_ENVIADO
    if len(data_ENVIADO) > 0:
        data_ENVIADO = np.array(data_ENVIADO)
        x,y = data_ENVIADO.T
        plt.scatter(x,y, s=1, c='b', label="Enviado")

    plt.title (u"Envio de posicionamento dos clientes para o UAV conectado")
    plt.xlabel(u"Tempo (s)")
    plt.ylabel(u"Cliente")
    lgd = plt.legend(loc='upper center', bbox_to_anchor=(0.5, 1.16), fancybox=True, shadow=True, ncol=5)

    plt.savefig(main_path+custo+'/client_packet.svg', bbox_extra_artists=(lgd,), bbox_inches='tight')
    plt.savefig(main_path+custo+'/client_packet.eps', bbox_extra_artists=(lgd,), bbox_inches='tight')
    plt.savefig(main_path+custo+'/client_packet.png', bbox_extra_artists=(lgd,), bbox_inches='tight')
    # plt.show()
    plt.close()
