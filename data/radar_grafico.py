import matplotlib.pyplot as plt
import pandas as pd
from math import pi
import numpy as np
import sys

scenario = sys.argv[2]
teste = False

time = sys.argv[1]

f_cen = open('./scratch/wifi/data/"+scenario+"/radar_uav_loc_'+'etapa/'+time+'.txt','r')
# f_cen = open('radar_uav_loc_0.txt','r')
data = {}
# read LOCs ids
line = f_cen.readline().strip()
locs = ['Loc'+x for x in line.split(',')]
if teste:
    print ('locs: '+str(locs))
# read UAVs ids
line = f_cen.readline().strip()
uavs = [x for x in line.split(',')]
data['group'] = uavs[:]
if teste:
    print ('uavs: '+str(uavs))
cont = 0
for line in f_cen:
    data[locs[cont]] = [float(x) for x in line.split(',')]
    cont = cont + 1
if teste:
    print ('data: '+str(data))
f_cen.close()

# Set data
df = pd.DataFrame(data)


# ------- PART 1: Create background
# number of variable
categories=locs
N = len(categories)

# What will be the angle of each axis in the plot? (we divide the plot / number of variable)
angles = [n / float(N) * 2 * pi for n in range(N)]
angles += angles[:1]

# Initialise the spider plot
ax = plt.subplot(111, polar=True)

# If you want the first axis to be on top:
ax.set_theta_offset(pi / 2)
ax.set_theta_direction(-1)

# Draw one axe per variable + add labels labels yet
plt.xticks(angles[:-1], categories)

# Draw ylabels
ax.set_rlabel_position(0)
# plt.yticks([10,20,30], ["10","20","30"], color="grey", size=7)
# plt.ylim(0,3)


# ------- PART 2: Add plots

# Plot each individual = each line of the data
# I don't do a loop, because plotting more than 3 groups makes the chart unreadable

for i in range(0, len(locs)-1):
    values=df.loc[i].drop('group').values.flatten().tolist()
    values += values[:1]
    if teste:
        print values
    ax.plot(angles, values, linewidth=1, linestyle='solid', label=str('UAV'+str(uavs[i])))
    ax.fill(angles, values, 'b', alpha=0.1)

# Add legend
plt.legend(loc='upper right', bbox_to_anchor=(0.1, 0.1))

plt.savefig('./scratch/wifi/data/da/radar_uav_loc_'+'etapa/'+time+'.svg')
# plt.savefig('teste.svg')
