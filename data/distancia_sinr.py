# -*- coding: UTF-8 -*-
# libraries and data
import numpy as np

d0 = 1.0 # m distancia minima
b = 3.7 # outdoor - tem alto impacto, quanto menor maior a distância de alcance
X = 0.0 # sombramento
fs = 0.0008 # fator de sobreposicao
N0 = 10e-9 # W/Hz ruido
B = 2e7 # 20MHz banda
co = 3e8 # comprimento de onda M
pt = 30.0 # dbm potencia de transmissao
gain = 4.0 # dbi ganho

def busca(fc, sinr_min):
    lamb = co/fc # lambda   
    pl_ref = 20*np.log10(4*3.141516*d0/lamb) # db path loss
    pr_ref = pt + gain + gain - pl_ref # dbm potencia recebida

    def sinrFromDistance (d):
        pl = 10*np.log10(d/d0)*b # db - simplified path loss Goldsmith 2.5 chapter
        # print pl
        pr = pr_ref - pl # dbm
        # print pr
        # it = fs*pr # dbm 
        # print it
        pr_W = dbmToWatts(pr)
        # print pr_W
        it_W = fs*pr_W
        # print it_W
        sinr_W = pr_W / (it_W + (N0*B)) 
        # print sinr_W
        return (wattsToDbm(sinr_W), pr)# dbm

    def dbmToWatts (dbm):
        return 10**((dbm-30)/10)

    def wattsToDbm (watts):
        return 10*np.log10(watts)+30

    d_max = 0
    d_anterior = 0

    # print "Iniciando busca..."
    for d in np.arange(1, 2000, 10):
        # print float(d)
        (sinr,pr) = sinrFromDistance(float(d))
        # print sinr
        if sinr == sinr_min:
            print "A distãncia "+str(d)+"m bate com a SINR "+str(sinr_min)+" dBm potencia recebida foi "+str(pr)
            return
        elif sinr < sinr_min:
            d_max = d # fixa a distancia máxima para filtro com pasos menores
            break
        else:
            d_anterior = d

    # print "Filtrando com passos menores d_anterior: " + str(d_anterior) + " d_max: "+str(d_max)
    for d in np.arange(d_anterior, d_max, 1):
        # print float(d)
        (sinr,pr) = sinrFromDistance(float(d))
        # print sinr
        if int(sinr) == sinr_min:
            print "A distãncia "+str(d)+"m bate com a SINR "+str(sinr_min)+" dBm potencia recebida foi "+str(pr)
            return 
        else:
            d_anterior = d
        
    for d in np.arange(d_anterior, d_max, 0.1):
        # print float(d)
        (sinr,pr) = sinrFromDistance(float(d))
        # print sinr
        if int(sinr) == sinr_min:
            print "A distãncia "+str(d)+"m bate com a SINR "+str(sinr_min)+" dBm potencia recebida foi "+str(pr)
            return 
        else:
            d_anterior = d

    print "Nenhuma distância!?"


# uav
print "UAV"
busca(5e9, -92)
# client
print "CLIENT"
busca(2.4e9, -93)