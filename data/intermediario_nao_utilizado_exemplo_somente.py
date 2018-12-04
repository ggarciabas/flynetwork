
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
    print "Executando intermediario "+str(time)
    # validando existencia de arquivos
    if len(glob.glob(main_path+time+'/bat_.eps')) == 0:
        return
    if len(glob.glob(main_path+time+'/cli_.eps')) == 0:
        return
    if len(glob.glob(main_path+time+'/bij_.eps')) == 0:
        return

    bat_file = os.path.basename(glob.glob(main_path+time+'/bat_.eps')[-1])
    bij_file = os.path.basename(glob.glob(main_path+time+'/bij_.eps')[-1])
    cli_file = os.path.basename(glob.glob(main_path+time+'/cli_.eps')[-1])

    if teste:
        print bat_file
        print bij_file
        print cli_file

    #copiar imagens para a pasta
    copyfile(main_path+time+'/'+cli_file, main_path+'../'+folder+'/'+str(time)+'_'+cli_file)
    copyfile(main_path+time+'/'+bij_file, main_path+'../'+folder+'/'+str(time)+'_'+bij_file)
    copyfile(main_path+time+'/'+bat_file, main_path+'../'+folder+'/'+str(time)+'_'+bat_file)


    if len(glob.glob(main_path+time+'/dist_.eps')) == 0:
        if len(glob.glob(main_path+time+'/mij_*_.eps')) == 0:
            return
        list_file=[]
        for file_path in glob.glob(main_path+time+'/mij_*_.eps'):
            file_name = os.path.basename(file_path)
            list_file.append(file_name)

        if len(list_file) == 0:
            return

        list_file = np.array(list_file)
        list_file.sort()

        mij_file = list_file[-1]
        if teste:
            print (mij_file)
        copyfile(main_path+time+'/'+mij_file, main_path+'../'+folder+'/'+str(time)+'_mij.eps')
        f_file = open(main_path+'../'+folder+'/slide_intermediario_'+str(time)+'.tex', 'w')
        f_file.write("""\\begin{frame}{"""+title+"""}
            \\begin{columns}
                \\begin{column}{0.45\\textwidth}
                   \\begin{figure}[!htb]
                        \\includegraphics[width=\\textwidth]{"""+folder+'/'+str(time)+'_'+bij_file+"""}
                    \\end{figure}
                    \\vspace{-0.5cm}
                   \\begin{figure}[!htb]
                        \\includegraphics[width=\\textwidth]{"""+folder+'/'+str(time)+"""_mij.eps}
                    \\end{figure}
                \\end{column}
                \\begin{column}{0.45\\textwidth}
                   \\begin{figure}[!htb]
                        \\includegraphics[width=\\textwidth]{"""+folder+'/'+str(time)+'_'+bat_file+"""}
                    \\end{figure}
                    \\vspace{-0.5cm}
                   \\begin{figure}[!htb]
                        \\includegraphics[width=\\textwidth]{"""+folder+'/'+str(time)+'_'+cli_file+"""}
                    \\end{figure}
                \\end{column}
            \\end{columns}
        \\end{frame}""")
        f_file.close()
    else:
        dist_file = os.path.basename(glob.glob(main_path+time+'/dist_.eps')[-1])
        if teste:
            print dist_file
        copyfile(main_path+time+'/'+dist_file, main_path+'../'+folder+'/'+str(time)+'_'+dist_file)
        f_file = open(main_path+'../'+folder+'/slide_intermediario_'+str(time)+'.tex', 'w')
        f_file.write("""\\begin{frame}{"""+title+"""}
            \\begin{columns}
                \\begin{column}{0.45\\textwidth}
                   \\begin{figure}[!htb]
                        \\includegraphics[width=\\textwidth]{"""+folder+'/'+str(time)+'_'+bat_file+"""}
                    \\end{figure}
                    \\vspace{-0.5cm}
                   \\begin{figure}[!htb]
                        \\includegraphics[width=\\textwidth]{"""+folder+'/'+str(time)+'_'+cli_file+"""}
                    \\end{figure}
                \\end{column}
                \\begin{column}{0.45\\textwidth}
                   \\begin{figure}[!htb]
                        \\includegraphics[width=\\textwidth]{"""+folder+'/'+str(time)+'_'+bij_file+"""}
                    \\end{figure}
                    \\vspace{-0.5cm}
                   \\begin{figure}[!htb]
                        \\includegraphics[width=\\textwidth]{"""+folder+'/'+str(time)+'_'+dist_file+"""}
                    \\end{figure}
                \\end{column}
            \\end{columns}
        \\end{frame}""")
        f_file.close()
