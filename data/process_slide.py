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
from shutil import copyfile
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

if not os.path.exists(main_path+'../slide/'+scenario):
    os.mkdir(main_path+'../slide/'+scenario)

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
    if not os.path.exists(main_path+'../slide/'+scenario+'/'+folder):
        os.mkdir(main_path+'../slide/'+scenario+'/'+folder)

    file = open(main_path+'..'+'/slide/'+scenario+'/slide_'+folder+'.tex', 'w')

    print folder

    for time in list_folder:
        print main_path+folder+'/etapa/'+str(time)+'/'
        # validando existencia de arquivos
        print glob.glob(main_path+folder+'/etapa/'+str(time)+'/da_loc_cpp_*.eps')
        if len(glob.glob(main_path+folder+'/etapa/'+str(time)+'/da_loc_cpp_*.eps')) == 0:
            print "exit da_loc_cpp_"
            exit()
        if len(glob.glob(main_path+folder+'/etapa/'+str(time)+'/location.eps')) == 0:
            print "exit location"
            exit()
        if len(glob.glob(main_path+folder+'/etapa/'+str(time)+'/prop_custo_*.eps')) == 0:
            print "exit custo_"
            exit()
        if len(glob.glob(main_path+folder+'/etapa/'+str(time)+'/ale_custo_*.eps')) == 0:
            print "exit ale_custo_"
            exit()
        if len(glob.glob(main_path+folder+'/etapa/'+str(time)+'/seq_custo_*.eps')) == 0:
            print "exit seq_custo_"
            exit()
        if len(glob.glob(main_path+folder+'/etapa/'+str(time)+'/exaustive_custo_*.eps')) == 0:
            print "exit exaustive_custo_"
            exit()
        if len(glob.glob(main_path+folder+'/etapa/'+str(time)+'/comp_custo_*.eps')) == 0:
            print "exit comp_custo_"
            exit()

        da_loc_file = os.path.basename(glob.glob(main_path+folder+'/etapa/'+str(time)+'/da_loc_cpp_*.eps')[-1])
        location_file = os.path.basename(glob.glob(main_path+folder+'/etapa/'+str(time)+'/location.eps')[-1])
        comp_custo_file = os.path.basename(glob.glob(main_path+folder+'/etapa/'+str(time)+'/comp_custo_*.eps')[-1])
        custo_file = os.path.basename(glob.glob(main_path+folder+'/etapa/'+str(time)+'/prop_custo_*.eps')[-1])
        ale_file = os.path.basename(glob.glob(main_path+folder+'/etapa/'+str(time)+'/ale_custo_*.eps')[-1])
        seq_file = os.path.basename(glob.glob(main_path+folder+'/etapa/'+str(time)+'/seq_custo_*.eps')[-1])
        exh_file = os.path.basename(glob.glob(main_path+folder+'/etapa/'+str(time)+'/exaustive_custo_*.eps')[-1])

        if teste:
            print da_loc_file
            print location_file
            print custo_file
            print exh_file
            print seq_file
            print ale_file

        #copiar imagens para a pasta
        copyfile(main_path+folder+'/etapa/'+str(time)+'/'+da_loc_file, main_path+folder+'/../../slide/'+scenario+'/'+folder+'/'+str(time)+'_'+da_loc_file)
        copyfile(main_path+folder+'/etapa/'+str(time)+'/'+location_file, main_path+folder+'/../../slide/'+scenario+'/'+folder+'/'+str(time)+'_'+location_file)
        #copiar imagens para a pasta
        copyfile(main_path+folder+'/etapa/'+str(time)+'/'+custo_file, main_path+folder+'/../../slide/'+scenario+'/'+folder+'/'+str(time)+'_'+custo_file)
        copyfile(main_path+folder+'/etapa/'+str(time)+'/'+ale_file, main_path+folder+'/../../slide/'+scenario+'/'+folder+'/'+str(time)+'_'+ale_file)
        copyfile(main_path+folder+'/etapa/'+str(time)+'/'+seq_file, main_path+folder+'/../../slide/'+scenario+'/'+folder+'/'+str(time)+'_'+seq_file)
        copyfile(main_path+folder+'/etapa/'+str(time)+'/'+exh_file, main_path+folder+'/../../slide/'+scenario+'/'+folder+'/'+str(time)+'_'+exh_file)
        copyfile(main_path+folder+'/etapa/'+str(time)+'/'+comp_custo_file, main_path+folder+'/../../slide/'+scenario+'/'+folder+'/'+str(time)+'_'+comp_custo_file)

        f_file = open(main_path+folder+'/../../slide/'+scenario+'/'+folder+'/slide_'+str(time)+'.tex', 'w')
        f_file.write("""\\begin{frame}{"""+title+' - Etapa @'+str(time)+"""}
            \\begin{columns}
                \\begin{column}{0.45\\textwidth}
                \\begin{figure}[!htb]
                        \\includegraphics[width=\\textwidth]{"""+folder+'/'+str(time)+'_'+da_loc_file+"""}
                    \\end{figure}
                \\end{column}
                \\begin{column}{0.45\\textwidth}
                \\begin{figure}[!htb]
                        \\includegraphics[width=\\textwidth]{"""+folder+'/'+str(time)+'_'+location_file+"""}
                    \\end{figure}
                \\end{column}
            \\end{columns}
        \\end{frame}""")
        f_file.write("""\n\\begin{frame}{"""+title+' - Etapa @'+str(time)+"""}
            \\begin{columns}
                \\begin{column}{0.45\\textwidth}
                \\begin{figure}[!htb]
                        \\includegraphics[width=\\textwidth]{"""+folder+'/'+str(time)+'_'+custo_file+"""}
                    \\end{figure}
                    \\vspace{-0.5cm}
                \\begin{figure}[!htb]
                        \\includegraphics[width=\\textwidth]{"""+folder+'/'+str(time)+'_'+ale_file+"""}
                    \\end{figure}
                \\end{column}
                \\begin{column}{0.45\\textwidth}
                \\begin{figure}[!htb]
                        \\includegraphics[width=\\textwidth]{"""+folder+'/'+str(time)+'_'+seq_file+"""}
                    \\end{figure}
                    \\vspace{-0.5cm}
                \\begin{figure}[!htb]
                        \\includegraphics[width=\\textwidth]{"""+folder+'/'+str(time)+'_'+exh_file+"""}
                    \\end{figure}
                \\end{column}
            \\end{columns}
        \\end{frame}""")
        f_file.write("""\n\\begin{frame}{"""+title+' - Etapa @'+str(time)+"""}
            \\begin{figure}[!htb]
                \\includegraphics[width=\\textwidth]{"""+folder+'/'+str(time)+'_'+comp_custo_file+"""}
            \\end{figure}
        \\end{frame}""")
        f_file.close()
        file.write("\\input{"+folder+'/slide_'+str(time)+'.tex}\n')

    copyfile(main_path+folder+'/client_packet.eps', main_path+folder+'/../../slide/'+scenario+'/'+folder+'/exh_'+folder+'.eps')
    file.write("""\\begin{frame}{"""+title+' - Comparativo exaustivo sequencial aleatorio'+"""}
        \\begin{figure}[!htb]
            \\includegraphics[width=0.9\\textwidth]{"""+folder+"""/exh_'+folder+'.eps}
        \\end{figure}
    \\end{frame}""")

    copyfile(main_path+folder+'/client_packet.eps', main_path+folder+'/../../slide/'+scenario+'/'+folder+'/client_packet.eps')
    file.write("""\\begin{frame}{"""+title+' - Envio de pacotes pelo cliente'+"""}
        \\begin{figure}[!htb]
            \\includegraphics[width=0.9\\textwidth]{"""+folder+"""/client_packet.eps}
        \\end{figure}
    \\end{frame}""")

    # # UAV LOC Slide
    # for time in list_folder:
    #     uav_loc_slide.slide (str(time), main_path+custo+'/', teste, title, folder, scenario)
    #     if len(glob.glob(main_path+'../slide/'+scenario+'/'+folder+'/slide_'+str(time)+'.tex'))>0:
    #         file.write("\\input{"+folder+'/'+os.path.basename(glob.glob(main_path+'../slide/'+scenario+'/'+folder+'/slide_'+str(time)+'.tex')[0])+"}\n")

    # # UAV BATTERY SLIDES
    # uav_bat_slide.slide(main_path+custo+'/', teste, title, folder, list_folder, scenario)
    # if len(glob.glob(main_path+'../slide/'+scenario+'/'+folder+'/slide_uav_energy_threshold.tex'))>0:
    #     file.write("\\input{"+folder+'/'+os.path.basename(glob.glob(main_path+'../slide/'+scenario+'/'+folder+'/slide_uav_energy_threshold.tex')[0])+"}\n")
    # if len(glob.glob(main_path+'../slide/'+scenario+'/'+folder+'/slide_uav_hover.tex'))>0:
    #     file.write("\\input{"+folder+'/'+os.path.basename(glob.glob(main_path+'../slide/'+scenario+'/'+folder+'/slide_uav_hover.tex')[0])+"}\n")
    # if len(glob.glob(main_path+'../slide/'+scenario+'/'+folder+'/slide_uav_hover_acum_uav.tex'))>0:
    #     file.write("\\input{"+folder+'/'+os.path.basename(glob.glob(main_path+'../slide/'+scenario+'/'+folder+'/slide_uav_hover_acum_uav.tex')[0])+"}\n")
    # if len(glob.glob(main_path+'../slide/'+scenario+'/'+folder+'/slide_uav_move.tex'))>0:
    #     file.write("\\input{"+folder+'/'+os.path.basename(glob.glob(main_path+'../slide/'+scenario+'/'+folder+'/slide_uav_move.tex')[0])+"}\n")
    # if len(glob.glob(main_path+'../slide/'+scenario+'/'+folder+'/slide_uav_move_acum_uav.tex'))>0:
    #     file.write("\\input{"+folder+'/'+os.path.basename(glob.glob(main_path+'../slide/'+scenario+'/'+folder+'/slide_uav_move.tex')[0])+"}\n")
    # if len(glob.glob(main_path+'../slide/'+scenario+'/'+folder+'/slide_uav_remaining_energy.tex'))>0:
    #     file.write("\\input{"+folder+'/'+os.path.basename(glob.glob(main_path+'../slide/'+scenario+'/'+folder+'/slide_uav_remaining_energy.tex')[0])+"}\n")
    # if len(glob.glob(main_path+'../slide/'+scenario+'/'+folder+'/slide_uav_remaining_energy_time.tex'))>0:
    #     file.write("\\input{"+folder+'/'+os.path.basename(glob.glob(main_path+'../slide/'+scenario+'/'+folder+'/slide_uav_remaining_energy_time.tex')[0])+"}\n")

    # # DA LOCALIZACAO CENARIOS
    # da_location_slide.slide(main_path+custo+'/', teste, title, folder, list_folder, scenario)
    # if len(glob.glob(main_path+'../slide/'+scenario+'/'+folder+'/slide_da_location.tex'))>0:
    #     file.write("\\input{"+folder+'/'+os.path.basename(glob.glob(main_path+'../slide/'+scenario+'/'+folder+'/slide_da_location.tex')[0])+"}\n")


    file.close()
