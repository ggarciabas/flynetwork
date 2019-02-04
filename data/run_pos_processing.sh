# $1 - True or False (debug)
# $2 - Scenario (name)

echo "General Graphic"
python general_graphic.py $1 $2
echo "uav removed Energy"
python uav_removed_energy.py $1 $2
# echo "connected cli"
# python connected_cli.py $1 $2

