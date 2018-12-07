#!/usr/bin/python
# -*- coding: UTF-8 -*-
# libraries and data
import uav_loc_slide
import intermediario
import glob
import os
import sys
import numpy as np

teste = True
if sys.argv[1] == "False":
    teste = False

scenario = sys.argv[2]

main_path = "./scratch/flynetwork/data/output/"+scenario+"/"
if teste :
    main_path = "./output/"+scenario+"/"

if not os.path.exists(main_path+'../slide'):
    os.mkdir(main_path+'../slide')

custos = ["custo_1", "custo_2", "custo_3", "custo_4"]
for custo in custos:
    title = ""
    if custo == "custo_1":
        title = "Custo 1"
    elif custo == "custo_2":
        title = "Custo 2"
    elif custo == "custo_3":
        title = "Custo 3"
    elif custo == "custo_4":
        title = "Custo 4"
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

    for time in list_folder:
        uav_loc_slide.slide (str(time), main_path+custo+'/', teste, title, folder)

    file = open(main_path+'..'+'/slide/slide_'+folder+'.tex', 'w')
    for time in list_folder:
        if len(glob.glob(main_path+'../slide/'+folder+'/slide_'+str(time)+'.tex'))>0:
            file.write("\\input{"+folder+'/'+os.path.basename(glob.glob(main_path+'../slide/'+folder+'/slide_'+str(time)+'.tex')[0])+"}\n")
    file.close()
