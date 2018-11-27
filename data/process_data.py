
# -*- coding: UTF-8 -*-
# libraries and data
import battery_mov
import battery_hover
import battery_all
import uav_loc_graphic
import bij_graphic
import mij_graphic
import moving_graphic
import initial_scenario
import cli_graphic
import bat_graphic
import client
import dist_graphic
import location
import glob
import os
import sys
import numpy as np

raio_uav = 59.8976798195
raio_cli = 108.295

teste = False
if sys.argv[1] == "False":
    teste = False

scenario = sys.argv[2]

main_path = "./output/"+scenario+"/"
# if teste :
#     main_path = "./output/"+scenario+"/"

list_folder = []

if len(sys.argv) == 4: # folder number
    list_folder.append(int(sys.argv[3]))
else:
    for folder_name in glob.glob(main_path+'*/'):
        list_folder.append(int(os.path.dirname(folder_name).split('/')[-1]))

if teste:
    print (list_folder)

list_folder = np.array(list_folder)
list_folder.sort()

uav_mov = {}
uavs_id = {}

# # Initial scenario
# initial_scenario.scenario(str(list_folder[0]), main_path, teste)
#
# # Client
# for time in list_folder:
#     client.scenario (str(time), main_path, teste)
#
# activated = {}
# file_ac = open(main_path+'uav_network_log.txt', 'r')
# for line in file_ac:
#     if teste:
#         print line
#     sep = [x for x in line.split(',')]
#
#     if not (str("UAV "+sep[1]) in activated):
#         activated[str("UAV "+sep[1])] = {}
#
#     if int(sep[2]) == 1:
#         activated[str("UAV "+sep[1])][int(sep[0])] = 'k'
#     elif int(sep[2]) == 0:
#         activated[str("UAV "+sep[1])][int(sep[0])] = 'r'
#
# if teste:
#     print activated
# t_ini = -1
# all_uav = []
# # Bij
# for time in list_folder:
#     if teste:
#         print time
#     (uavs_id, uav_mov) = bij_graphic.bij(str(time), main_path, teste)
#     if teste:
#         print uavs_id
#     # Battery
#     if t_ini > -1:
#         # battery_mov.battery(main_path, teste, uavs_id, t_ini, time)
#         battery_hover.battery(main_path, teste, uavs_id, t_ini, time, activated, time)
#         battery_all.battery(main_path, teste, uavs_id, t_ini, time, activated, time)
#         t_ini = time
#     else:
#         t_ini = -0.5
#
#     all_uav.extend(uavs_id[:])
#
# if teste:
#     print all_uav
# battery_all.battery(main_path, teste, all_uav, 0, t_ini+10, activated, 'all')
# battery_mov.battery(main_path, teste, [], 0, t_ini+10, activated, 'all')
# battery_hover.battery(main_path, teste, all_uav, 0, t_ini+10, activated, 'all')
#
# # Dist
# for time in list_folder:
#     dist_graphic.dist(str(time), main_path, teste)

# Cli
for time in list_folder:
    cli_graphic.cli(str(time), main_path, teste)

# Bat
for time in list_folder:
    bat_graphic.bat(str(time), main_path, teste)

# Location
for time in list_folder:
    location.scenario (str(time), main_path, teste)

# Uav Loc
for time in list_folder:
    uav_loc_graphic.uav_loc (str(time), main_path, teste, raio_cli, raio_uav)

# Mij
for time in list_folder:
    mij_graphic.mij(str(time), main_path, teste)

# Moving
moving_graphic.moving(main_path, teste, uav_mov)
