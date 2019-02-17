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
    data_ENVIADO = []
    data_RECEBIDO = []

    file_c = open(main_path+custo+'/client_data.txt', 'r')
    for line in file_c:
        data = [x for x in line.split(' ')]
        if data[1] == "ENVIADO\n":
            data_ENVIADO.append([float(data[0]), "TX"])
        elif data[1] == "RECEBIDO\n":
            data_NAOCONECTADO.append([float(data[0]), "RX"])

    print data_ENVIADO
    if len(data_ENVIADO) > 0:
        data_ENVIADO = np.array(data_ENVIADO)
        x,y = data_ENVIADO.T
        plt.scatter(x,y, s=1, c='r')

    print data_RECEBIDO
    if len(data_RECEBIDO) > 0:
        data_RECEBIDO = np.array(data_RECEBIDO)
        x,y = data_RECEBIDO.T
        plt.scatter(x,y, s=1, c='b')



    plt.title (u"Envio e recebimento de aplicações cliente")
    plt.xlabel(u"Tempo (s)")
    plt.ylabel(u"TX/RX")

    plt.savefig(main_path+custo+'/client_data.svg', bbox_extra_artists=(lgd,), bbox_inches='tight')
    plt.savefig(main_path+custo+'/client_data.eps', bbox_extra_artists=(lgd,), bbox_inches='tight')
    plt.savefig(main_path+custo+'/client_data.png', bbox_extra_artists=(lgd,), bbox_inches='tight')
    # plt.show()
    plt.close()
