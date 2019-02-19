
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

def slide (main_path, teste, title, folder, list_folder, scenario):
    print "Executando da location slide"
    if len(glob.glob(main_path+'python/files.txt')) > 0:
        f_file = open(main_path+'../../slide/'+scenario+'/'+folder+'/slide_da_location.tex', 'w')
        files_da = open(main_path+'python/files.txt', 'r')
        for line in files_da:
            copyfile(main_path+'python/'+line[:-1], main_path+'../../slide/'+scenario+'/'+folder+'/'+line[:-1])
            f_file.write("""\\begin{frame}{"""+title+""" - DA Location}
                \\begin{figure}[!htb]
                     \\includegraphics[width=\\textwidth]{"""+folder+'/'+line[:-1]+"""}
                 \\end{figure}
            \\end{frame}""")
        files_da.close()
        f_file.close()
