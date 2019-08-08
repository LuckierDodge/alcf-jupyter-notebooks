#!/bin/bash
#COBALT -t 60
#COBALT -n 1
#COBALT -q debug
#COBALT -O ./output/$jobid
#COBALT -M ryan.lewis@anl.gov

year=2016
month=1
day=1

for file in /gpfs/mira-fs0/logs/darshan/mira/$year/$month/$day/*
do
	./process-log $file ../data/darshan/${year}_${month}_${day}_$(basename $file).csv
done
