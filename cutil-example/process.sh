#!/bin/bash

year=$1
month=$2
day=$3

if [ -d /gpfs/mira-fs0/logs/darshan/mira/$year/$month/$day/ ]; then
	mkdir -p /lus/theta-fs0/projects/AMASE/rlewis/darshan/$year/$month/$day/
	for file in /gpfs/mira-fs0/logs/darshan/mira/$year/$month/$day/*.darshan.gz
	do
		echo /home/rlewis/repos/jupyter-notebooks/cutil-example/process-log $file /lus/theta-fs0/projects/AMASE/rlewis/darshan/$year/$month/$day/$(basename $file).csv
	done | xargs -I CMD --max-procs=100 bash -c CMD
fi
