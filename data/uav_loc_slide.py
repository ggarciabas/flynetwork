
# -*- coding: UTF-8 -*-
# libraries and data
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import seaborn as sns
from shutil import copyfile
import glob
import os
import sys

def slide (time, main_path, teste, title, folder):
    print "Executando uav loc "+str(time)
    # validando existencia de arquivos
    if len(glob.glob(main_path+'etapa/'+time+'/f_mij_.eps')) == 0:
        return
    if len(glob.glob(main_path+'etapa/'+time+'/bij_.eps')) == 0:
        return
    if len(glob.glob(main_path+'etapa/'+time+'/uav_loc.eps')) == 0:
        return

    mij_file = os.path.basename(glob.glob(main_path+'etapa/'+time+'/f_mij_.eps')[-1])
    bij_file = os.path.basename(glob.glob(main_path+'etapa/'+time+'/bij_.eps')[-1])
    uav_loc_file = os.path.basename(glob.glob(main_path+'etapa/'+time+'/uav_loc.eps')[-1])

    if teste:
        print mij_file
        print bij_file
        print uav_loc_file

    #copiar imagens para a pasta
    copyfile(main_path+'etapa/'+time+'/'+mij_file, main_path+'../../slide/'+folder+'/'+str(time)+'_'+mij_file)
    copyfile(main_path+'etapa/'+time+'/'+bij_file, main_path+'../../slide/'+folder+'/'+str(time)+'_'+bij_file)
    copyfile(main_path+'etapa/'+time+'/'+uav_loc_file, main_path+'../../slide/'+folder+'/'+str(time)+'_'+uav_loc_file)

    f_file = open(main_path+'../../slide/'+folder+'/slide_'+str(time)+'.tex', 'w')
    f_file.write("""\\begin{frame}{"""+title+"""}
        \\begin{columns}
            \\begin{column}{0.65\\textwidth}
               \\begin{figure}[!htb]
                    \\includegraphics[width=\\textwidth]{"""+folder+'/'+str(time)+'_'+uav_loc_file+"""}
                \\end{figure}
            \\end{column}
            \\begin{column}{0.45\\textwidth}
               \\begin{figure}[!htb]
                    \\includegraphics[width=\\textwidth]{"""+folder+'/'+str(time)+'_'+bij_file+"""}
                \\end{figure}
                \\vspace{-0.5cm}
               \\begin{figure}[!htb]
                    \\includegraphics[width=\\textwidth]{"""+folder+'/'+str(time)+'_'+mij_file+"""}
                \\end{figure}
            \\end{column}
        \\end{columns}
    \\end{frame}""")
    f_file.close()
