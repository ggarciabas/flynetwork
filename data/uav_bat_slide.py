
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

def slide (main_path, teste, title, folder):
    print "Executando uav bat slide"
    # ENERGY THRESHOLD -- validando existencia de arquivos
    if len(glob.glob(main_path+'uav_energy_threshold/uav_energy_threshold_.eps')) == 0:
        return

    uet_file = os.path.basename(glob.glob(main_path+'uav_energy_threshold/uav_energy_threshold_.eps')[-1])

    #copiar imagens para a pasta
    copyfile(main_path+'uav_energy_threshold/'+uet_file, main_path+'../../slide/'+folder+'/'+uet_file)

    if teste:
        print uet_file

    f_file = open(main_path+'../../slide/'+folder+'/slide_uav_energy_threshold.tex', 'w')
    f_file.write("""\\begin{frame}{"""+title+""" - Energy Threshold}
        \\begin{figure}[!htb]
             \\includegraphics[width=\\textwidth]{"""+folder+'/'+uet_file+"""}
         \\end{figure}
    \\end{frame}""")
    f_file.close()

    # HOVER -- validando existencia de arquivos
    if len(glob.glob(main_path+'uav_hover/uav_hover_acum_all.eps')) == 0:
        return

    uh_file = os.path.basename(glob.glob(main_path+'uav_hover/uav_hover_acum_all.eps')[-1])

    #copiar imagens para a pasta
    copyfile(main_path+'uav_hover/'+uh_file, main_path+'../../slide/'+folder+'/'+uh_file)

    if teste:
        print uh_file

    f_file = open(main_path+'../../slide/'+folder+'/slide_uav_hover.tex', 'w')
    f_file.write("""\\begin{frame}{"""+title+""" - Hover}
        \\begin{figure}[!htb]
             \\includegraphics[width=\\textwidth]{"""+folder+'/'+uh_file+"""}
         \\end{figure}
    \\end{frame}""")
    f_file.close()

    # MOVE -- validando existencia de arquivos
    if len(glob.glob(main_path+'uav_hover/uav_move_acum_all.eps')) == 0:
        return

    um_file = os.path.basename(glob.glob(main_path+'uav_move/uav_move_acum_all.eps')[-1])

    #copiar imagens para a pasta
    copyfile(main_path+'uav_move/'+uh_file, main_path+'../../slide/'+folder+'/'+um_file)

    if teste:
        print um_file

    f_file = open(main_path+'../../slide/'+folder+'/slide_uav_move.tex', 'w')
    f_file.write("""\\begin{frame}{"""+title+""" - Move}
        \\begin{figure}[!htb]
             \\includegraphics[width=\\textwidth]{"""+folder+'/'+um_file+"""}
         \\end{figure}
    \\end{frame}""")
    f_file.close()

    # REMAINING ENERGY -- validando existencia de arquivos
    if len(glob.glob(main_path+'uav_hover/uav_remaining_energy_all.eps')) == 0:
        return

    ure_file = os.path.basename(glob.glob(main_path+'uav_remaining_energy/uav_remaining_energy_all.eps')[-1])

    #copiar imagens para a pasta
    copyfile(main_path+'uav_remaining_energy/'+uh_file, main_path+'../../slide/'+folder+'/'+ure_file)

    if teste:
        print um_file

    f_file = open(main_path+'../../slide/'+folder+'/slide_uav_remaining_energy.tex', 'w')
    f_file.write("""\\begin{frame}{"""+title+""" - Reamining Energy}
        \\begin{figure}[!htb]
             \\includegraphics[width=\\textwidth]{"""+folder+'/'+ure_file+"""}
         \\end{figure}
    \\end{frame}""")
    f_file.close()
