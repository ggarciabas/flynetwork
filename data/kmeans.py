#!/usr/bin/env python
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import glob
import os
import sys
plt.rcParams['figure.figsize'] = (16, 9)
plt.style.use('ggplot')


'''
==========================================================
scikit-learn
==========================================================
'''

from sklearn.cluster import KMeans

teste = True
# if sys.argv[1] == "False":
#     teste = False
scenario = sys.argv[2]

custos = ["custo_1"] #, "custo_2", "custo_3", "custo_4"]
for custo in custos:
    main_path = "./output/"+scenario+"/"+custo+"/"
    list_folder = []
    if len(sys.argv) == 4: # folder number
        list_folder.append(int(sys.argv[3]))
    else:
        for folder_name in glob.glob(main_path+'etapa/*/'):
            if teste:
                print "folder_name: "+str(folder_name)
            list_folder.append(int(os.path.dirname(folder_name).split('/')[-1]))
    if teste:
        print (list_folder)
    list_folder = np.array(list_folder)
    list_folder.sort()
    for time in list_folder:
        path = main_path+'/etapa/'+str(time)+'/'
        f_cen = open(path+'cenario_in.txt','r')
        cen = f_cen.readline() # retirando tipo de propagacao utilizada
        total = f_cen.readline() # retirando total de clientes do arquivo
        lcentral = f_cen.readline() # read central position
        central = [float(x) for x in lcentral.split(',') if x.strip().isdigit()]
        nn = []
        nn.append([])
        nn.append([])
        for line in f_cen:
            # print line
            if line[0]!= '#':
                mylist = [x for x in line.split(',')]
                # print mylist
                nn[0].append(float(mylist[1]))
                nn[1].append(float(mylist[2]))
        f_cen.close()
        X = np.array([nn[0],nn[1]]).T # posicionamento dos usuarios

        print "X: "+str(X)
        wcss = []
        for i in range(1, X.shape[0]):
            kmeans = KMeans(n_clusters = i, init = 'random')
            kmeans.fit(X)
            y_kmeans = kmeans.predict(X)
            plt.clf()
            plt.scatter(X[:, 0], X[:,1], c=y_kmeans, label='Clientes')
            plt.scatter(kmeans.cluster_centers_[:, 0], kmeans.cluster_centers_[:, 1], c='black', alpha=0.5, label='UAVs')
            plt.title(u'Posicionamento dos UAVs K='+str(i))
            plt.xlabel('X(m)')
            plt.ylabel('Y(m)')
            plt.legend()

            plt.savefig(path+'kmeans_'+str(i)+'.svg')
            plt.savefig(path+'kmeans_'+str(i)+'.eps')
            plt.savefig(path+'kmeans_'+str(i)+'.png')

            if teste:
                print i,kmeans.inertia_
            wcss.append(kmeans.inertia_)

        plt.clf()
        plt.plot(range(1, X.shape[0]), wcss)
        plt.title('O Metodo Elbow')
        plt.xlabel('Numero de Clusters')
        plt.ylabel('WSS') # within cluster sum of squares

        plt.savefig(path+'kmeans_elbow.svg')
        plt.savefig(path+'kmeans_elbow.eps')
        plt.savefig(path+'kmeans_elbow.png')
