general:
	python general_graphic.py $(d) $(s)

exhaustive:
	python exhaustive_prop.py $(d) $(s) $(f=4)

packet:
	python client_packet.py $(d) $(s)

dhcp:
	python client_dhcp.py $(d) $(s)

data:
	python client_data.py $(d) $(s)

uav_removed:
	python uav_removed_energy.py $(d) $(s)

# make c=custo_3 e=625 m="./output/austin_30" t=True i=-2 da_loc
da_loc:
	python da_loc.py $(c) $(e) $(m) $(t) $(i)
