
# -*- coding: UTF-8 -*-
# libraries and data
import glob
import os
import sys
import numpy as np
import battery_mov
import battery_hover
import battery_all
import uav_loc_graphic
import bij_graphic
import mij_graphic
import moving_graphic
import battery_threshold
import initial_scenario
import client
import da_loc
import location
import glob
import os
import sys
import custo_graphic
import numpy as np

raio_uav = 59.8976798195
raio_cli = 108.295

teste = True
if sys.argv[1] == "False":
    teste = False
scenario = sys.argv[2]
seed = sys.argv[3]
protocol = sys.argv[4]
main_path = "./output/"+scenario+"/"+seed+"/"+protocol+"/"

for custo_name in glob.glob(main_path+'custo_*/'):
    custo = os.path.dirname(custo_name).split('/')[-1]
    if teste:
        print "Custo: "+custo
    list_folder = []
    if len(sys.argv) == 6: # folder number
        list_folder.append(int(sys.argv[5]))
    else:
        for folder_name in glob.glob(main_path+custo+'/etapa/*/'):
            list_folder.append(int(os.path.dirname(folder_name).split('/')[-1]))
    if teste:
        print 'list folder'
        print (list_folder)
    list_folder = np.array(list_folder)
    list_folder.sort()
    uav_mov = {}
    uavs_id = {}
    # Initial scenario
    initial_scenario.scenario(str(list_folder[0]), main_path+custo+'/', teste)
    activated = {}
    file_ac = open(main_path+custo+'/uav_network_log.txt', 'r')
    for line in file_ac:
        if teste:
            print line
        sep = [x for x in line.split(',')]

        if not (str("UAV "+sep[1]) in activated):
            activated[str("UAV "+sep[1])] = {}

        if int(sep[2]) == 1:
            activated[str("UAV "+sep[1])][float(sep[0])] = 'k'
        elif int(sep[2]) == 0:
            activated[str("UAV "+sep[1])][float(sep[0])] = 'r'
        elif int(sep[2]) == 2:
            activated[str("UAV "+sep[1])][float(sep[0])] = 'g'
    if teste:
        print activated
    t_ini = -1
    all_uav = []
    
    if teste:
        print "Iniciando ------"
    for etapa in list_folder:
        if teste:
            print main_path
            print custo
            print etapa
        da_loc.da_loc (custo, etapa, main_path, teste, -2) # somente a ultima iteracao
        if teste:
            print etapa
        # (uavs_id, uav_mov) = bij_graphic.bij(custo, str(etapa), main_path, teste)
        # custo_graphic.custo(custo, str(etapa), main_path, teste)
        # if teste:
        #     print uavs_id
        # Battery
        # if t_ini > -1:
        #     battery_hover.battery(main_path+custo+'/', teste, uavs_id, t_ini, etapa, activated, etapa)
        #     battery_all.battery(main_path+custo+'/', teste, uavs_id, t_ini, etapa, activated, etapa)
        #     t_ini = etapa
        # else:
        #     t_ini = -0.5
        # Client
        client.scenario (str(etapa), main_path+custo+'/', teste)
        # Location
        location.scenario (str(etapa), main_path+custo+'/', teste)
        # Uav Loc
        uav_loc_graphic.uav_loc (str(etapa), main_path+custo+'/', teste, raio_cli, raio_uav)
        # Mij
        # mij_graphic.mij(str(etapa), main_path+custo+'/', teste)
        # all_uav.extend(uavs_id[:])

    # if teste:
    #     print all_uav
    # battery_all.battery(main_path+custo+'/', teste, all_uav, 0, t_ini+10, activated, 'all')
    # battery_mov.battery(main_path+custo+'/', teste, [], 0, t_ini+10, activated, 'all')
    # battery_hover.battery(main_path+custo+'/', teste, [], 0, t_ini+10, activated, 'all')
    # battery_hover.battery(main_path+custo+'/', teste, all_uav, 0, t_ini+10, activated, 'all')
    # battery_threshold.battery(main_path+custo+'/', teste, all_uav, 0, t_ini+10, activated, '')
