import permutation


# string = "ABCD"
# n = len(string)
# a = list(string)
# permutation.permute(a, 0, n-1)

teste = True
if sys.argv[1] == "False":
    teste = False
scenario = sys.argv[2]
main_path = "./output/"+scenario+"/"
list_folder = []
if len(sys.argv) == 4: # folder number
    list_folder.append(int(sys.argv[3]))
else:
    for folder_name in glob.glob(main_path+'*/'):
        list_folder.append(int(os.path.dirname(folder_name).split('/')[-1]))
if teste:
    print (list_folder)
list_folder = np.array(list_folder)
list_folder.sort()

for time in list_folder:
    # read bij
    try:
        file = open(main_path+time+"/bij.txt", 'r')
    except IOError:
        return
    line = file.readline().strip()
    uavs_id = [int(x) for x in line.split(',')]
    line = file.readline().strip()
    locs_id = [int(x) for x in line.split(',')]
    data_bij = {}
    cont = 0
    for line in file:
        data_bij[uavs_id[cont]] = [float(x) for x in line.split(',')] # read row from file
        cont = cont + 1
    file.close()

    min_conf=locs_id[:]
    min_value=9999.0

    # thanks to: https://www.geeksforgeeks.org/write-a-c-program-to-print-all-permutations-of-a-given-string/
    def permute(uav_loc, start, end): # each positionof uav_loc corresponds to a UAV and each value in any position corresponds to the location, this relates an UAV to a Location
        if start==r:
            value = 0.0
            c=0
            for l in uav_loc:
                value = value + data_bij[c][l]
                c=c+1
            if value < min_value:
                min_value = value
                min_conf = uav_loc[:] # if do not use [:], both uav_loc and min_conf will corresponds to the same pointer in memory!
        else:
            for i in xrange(start,end+1):
                uav_loc[start], uav_loc[i] = uav_loc[i], uav_loc[start]
                permute(uav_loc, start+1, end)
                uav_loc[start], uav_loc[i] = uav_loc[i], uav_loc[start] # backtrack

    permute (min_conf[:], 0, len(min_conf)-1)

    f_file = open(main_path+time+"/exaustive.txt", 'w')
    for k in range(0,K,1):
        f_file.write(str(Yj[k][0])+','+str(Yj[k][1])+'\n')
        for u in uav_user_connected[k]:
            f_file.write(str(id[u])+' ')
        f_file.write('\n')
    f_file.close()
