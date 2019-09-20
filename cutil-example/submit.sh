#!/bin/bash
#COBALT -t 720
#COBALT -n 32
#COBALT -q default
#COBALT -O ./output/$jobid
#COBALT -M ryan.lewis@anl.gov
#COBALT -A AMASE

NODES=`cat $COBALT_NODEFILE | wc -l`

PROCS=$((NODES * 12))

mpirun -f $COBALT_NODEFILE -n $PROCS ./process-logs list.txt /lus/theta-fs0/projects/AMASE/rlewis/darshan/debug/2015

# year=$1
# month=$2
# 
# NODES=`cat $COBALT_NODEFILE`
# NODE_ARRAY=($NODES)
# 
# for day in {1..31}
# do
# 	if [ $day == 1 ]; then
# 		/home/rlewis/repos/jupyter-notebooks/cutil-example/process.sh $year $month $day &
# 	else
# 		let "index = ${day} - 1"
# 		ssh ${NODE_ARRAY[$index]} "/home/rlewis/repos/jupyter-notebooks/cutil-example/process.sh $year $month $day" &
# 	fi
# done
# wait
