#!/usr/bin/python
# -*- coding: UTF-8 -*-
# libraries and data
import uav_loc_slide
import uav_bat_slide
import da_location_slide
import glob
import os
import sys
import numpy as np
from custos_ativos import c_name # c_name

teste = True
if sys.argv[1] == "False":
    teste = False

scenario = sys.argv[2]

main_path = "./scratch/flynetwork/data/output/"+scenario+"/"
if teste :
    main_path = "./output/"+scenario+"/"

if not os.path.exists(main_path+'../slide'):
    os.mkdir(main_path+'../slide')

for custo_name in glob.glob(main_path+'custo_*/'):
    custo = os.path.dirname(custo_name).split('/')[-1]
    title = c_name[custo]
    folder = custo
    list_folder = []

    if int(sys.argv[3]) == -1: # folder number
        for folder_name in glob.glob(main_path+custo+'/etapa/*/'):
            list_folder.append(int(os.path.dirname(folder_name).split('/')[-1]))
    else:
        list_folder.append(int(sys.argv[3]))

    if teste:
        print (list_folder)

    list_folder = np.array(list_folder)
    list_folder.sort()

    # criar pasta TODO
    if not os.path.exists(main_path+'../slide/'+folder):
        os.mkdir(main_path+'../slide/'+folder)

    # UAV LOC Slide
    file = open(main_path+'..'+'/slide/slide_'+folder+'.tex', 'w')
    for time in list_folder:
        uav_loc_slide.slide (str(time), main_path+custo+'/', teste, title, folder)
        if len(glob.glob(main_path+'../slide/'+folder+'/slide_'+str(time)+'.tex'))>0:
            file.write("\\input{"+folder+'/'+os.path.basename(glob.glob(main_path+'../slide/'+folder+'/slide_'+str(time)+'.tex')[0])+"}\n")

    # UAV BATTERY SLIDES
    uav_bat_slide.slide(main_path+custo+'/', teste, title, folder, list_folder)
    if len(glob.glob(main_path+'../slide/'+folder+'/slide_uav_energy_threshold.tex'))>0:
        file.write("\\input{"+folder+'/'+os.path.basename(glob.glob(main_path+'../slide/'+folder+'/slide_uav_energy_threshold.tex')[0])+"}\n")
    if len(glob.glob(main_path+'../slide/'+folder+'/slide_uav_hover.tex'))>0:
        file.write("\\input{"+folder+'/'+os.path.basename(glob.glob(main_path+'../slide/'+folder+'/slide_uav_hover.tex')[0])+"}\n")
    if len(glob.glob(main_path+'../slide/'+folder+'/slide_uav_hover_acum_uav.tex'))>0:
        file.write("\\input{"+folder+'/'+os.path.basename(glob.glob(main_path+'../slide/'+folder+'/slide_uav_hover_acum_uav.tex')[0])+"}\n")
    if len(glob.glob(main_path+'../slide/'+folder+'/slide_uav_move.tex'))>0:
        file.write("\\input{"+folder+'/'+os.path.basename(glob.glob(main_path+'../slide/'+folder+'/slide_uav_move_acum_uav.tex')[0])+"}\n")
    if len(glob.glob(main_path+'../slide/'+folder+'/slide_uav_move_acum_uav.tex'))>0:
        file.write("\\input{"+folder+'/'+os.path.basename(glob.glob(main_path+'../slide/'+folder+'/slide_uav_move.tex')[0])+"}\n")
    if len(glob.glob(main_path+'../slide/'+folder+'/slide_uav_remaining_energy.tex'))>0:
        file.write("\\input{"+folder+'/'+os.path.basename(glob.glob(main_path+'../slide/'+folder+'/slide_uav_remaining_energy.tex')[0])+"}\n")
    if len(glob.glob(main_path+'../slide/'+folder+'/slide_uav_remaining_energy_time.tex'))>0:
        file.write("\\input{"+folder+'/'+os.path.basename(glob.glob(main_path+'../slide/'+folder+'/slide_uav_remaining_energy_time.tex')[0])+"}\n")

    # DA LOCALIZACAO CENARIOS
    da_location_slide.slide(main_path+custo+'/', teste, title, folder, list_folder)
    if len(glob.glob(main_path+'../slide/'+folder+'/slide_da_location.tex'))>0:
        file.write("\\input{"+folder+'/'+os.path.basename(glob.glob(main_path+'../slide/'+folder+'/slide_da_location.tex')[0])+"}\n")

    file.close()
