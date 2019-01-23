import numpy as np

d0 = 1
b = 3.22 # outdoor
X = 0 # sombramento
fs = 0.0008 # fator de sobreposicao
N0 = 10e-9 # W/Hz ruido
B = 2e7 # 20MHz banda

# uav
fc =fcCli 5e9 # frequencia Hz
co = 3e8 # comprimento de onda M
lamb = co/fc # lambda
pt = 30 # dbm potencia de transmissao
gain = 4 # dbi ganho
sinr_min = -93 # dbm 

pl_ref = 20*np.log10(4*3.141516*d0/lamb) # db path loss
pr_ref = pt + gain + gain - pl_ref # dbm potencia recebida

def sinrFromDistance (d):
    pl = 20*np.log10(d)*b+X # db
    pr = pr_ref - pl # dbm
    it = fs*pr # dbm 
    pr_W = dbmToWatts(pr)
    it_W = dbmToWatts(it)
    sinr_w = pr_W / (it_W + (N0*B)) 
    return wattsToDbm(sinr_w) # dbm

def dbmToWatts (dbm):
    return 10^((dbm-30)/10)

def wattsToDbm (watts):
    return 10*np.log10(watts)+30

d_max = 0
d_anterior = 0

print "Iniciando busca..."
for d in np.arange(1, 2000, 100):
    sinr = sinrFromDistance(d)
    if sinr == sinr_min:
        print "A distãncia "+str(d)+" bate com a SINR "+str(sinr_min)
        exit()
    elif sinr > sinr_min:
        d_max = d # fixa a distancia máxima para filtro com pasos menores
        break
    else
        d_atual = d

print "Filtrando com passos menores..."
for d in np.arange(d_atual, d_max, 1):
    sinr = sinrFromDistance(d)
    if sinr == sinr_min:
        print "A distãncia "+str(d)+" bate com a SINR "+str(sinr_min)
        exit()

print "Nenhuma distância!?"