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

title = sys.argv[4]
folder = sys.argv[5]

main_path = "./scratch/flynetwork/data/output/"+scenario+"/"
if teste :
    main_path = "./output/"+scenario+"/"

list_folder = []

if int(sys.argv[3]) == -1: # folder number
    for folder_name in glob.glob(main_path+'*/'):
        list_folder.append(int(os.path.dirname(folder_name).split('/')[-1]))
else:
    list_folder.append(int(sys.argv[3]))

if teste:
    print (list_folder)

list_folder = np.array(list_folder)
list_folder.sort()

# criar pasta TODO
if not os.path.exists(main_path+'../'+folder):
    os.mkdir(main_path+'../'+folder)

for time in list_folder:
    uav_loc_slide.slide (str(time), main_path, teste, title, folder)
    intermediario.slide (str(time), main_path, teste, title, folder)

file = open(main_path+'..'+'/slide_'+folder+'.tex', 'w')
for time in list_folder:
    if len(glob.glob(main_path+'../'+folder+'/slide_'+str(time)+'.tex'))>0:
        file.write("\\input{"+folder+'/'+os.path.basename(glob.glob(main_path+'../'+folder+'/slide_'+str(time)+'.tex')[0])+"}\n")
    if len(glob.glob(main_path+'../'+folder+'/slide_intermediario_'+str(time)+'.tex'))>0:
        file.write("\\input{"+folder+'/'+os.path.basename(glob.glob(main_path+'../'+folder+'/slide_intermediario_'+str(time)+'.tex')[0])+"}\n")
file.close()
