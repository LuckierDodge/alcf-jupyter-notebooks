#!/bin/bash
#COBALT -t 120
#COBALT -n 2
#COBALT -q debug
#COBALT -O ./output/$jobid
#COBALT -M ryan.lewis@anl.gov
#COBALT -A AMASE

year=$1
month=$2

NODES=`cat $COBALT_NODEFILE`
NODE_ARRAY=($NODES)

# day=2
# let "index = ${day} - 1"
# ssh ${NODE_ARRAY[$index]} pwd

for day in {1..2} #{1..31}
do
	if [ $day == 1 ]; then
		/home/rlewis/repos/jupyter-notebooks/cutil-example/process.sh $year $month $day &
	else
		let "index = ${day} - 1"
		ssh ${NODE_ARRAY[$index]} "/home/rlewis/repos/jupyter-notebooks/cutil-example/process.sh $year $month $day" &
	fi
done
wait
