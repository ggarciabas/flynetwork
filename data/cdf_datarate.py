import numpy as np
import matplotlib.pyplot as plt
import scipy.stats as stats
from collections import Counter

np.random.seed(17012019)

# comparar com as demais simulacoes: (DA puro e Kmeans)

def cdf_datarate_client (custo, etapa, main_path, teste):
    try:
        f_cen = open(main_path+'/'+custo+'/etapa/'+etapa+'/client_data_rate.txt','r')
    except IOError:
        exit()

    n_bins = 1
    data = []
    min_ = 100
    max_ = 0
    for line in f_cen:
        d = [x for x in line.split(',')] # read row from file
        data.append(float(d[1])) # pegando somente a taxa    
        if float(d[1])>max_:
            max_ = float(d[1])
        elif float(d[1]) < min_:
            min_ = float(d[1])


    print data

    fig, ax = plt.subplots(figsize=(8, 4))

    # Choose how many bins you want here
    num_bins = 20

    # Use the histogram function to bin the data
    counts, bin_edges = np.histogram(data, bins=num_bins, normed=True)

    # Now find the cdf
    cdf = np.cumsum(counts)

    # And finally plot the cdf
    plt.plot(bin_edges[1:], cdf)

    # tidy up the figure
    ax.grid(True)
    # ax.legend(loc='right')
    ax.set_title('Cumulative step histograms')
    ax.set_xlabel('Data Rate (Mbps)')
    ax.set_ylabel('Likelihood of occurrence')
    # ax.set_xlim([min_, max_])

    plt.show()

def pdf_datarate_client (custo, etapa, main_path, teste):
    try:
        f_cen = open(main_path+'/'+custo+'/etapa/'+etapa+'/client_data_rate.txt','r')
    except IOError:
        exit()

    data = []
    min_ = 100
    max_ = 0
    for line in f_cen:
        d = [x for x in line.split(',')] # read row from file
        data.append(float(d[1])) # pegando somente a taxa    
        if float(d[1])>max_:
            max_ = float(d[1])
        elif float(d[1]) < min_:
            min_ = float(d[1])

    if teste:
        print data

    # Compute histogram of Samples
    n, bins, patches = plt.hist(data, density=True, facecolor='g', edgecolor='red', alpha=0.75)

    plt.xlabel('Probability Density Function')
    plt.ylabel('Probability')
    plt.title('Data rate (Mbps)')

    plt.show()

# cdf_datarate_client ('custo_1', '65', './output/austin_30', False)
pdf_datarate_client ('custo_1', '65', './output/austin_30', False)