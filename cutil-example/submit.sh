#!/bin/bash
#COBALT -t 60
#COBALT -n 1
#COBALT -q debug
#COBALT -O ./output/$jobid

for file in /gpfs/mira-fs0/logs/darshan/mira/2016/1/1/*
do
	./process-log $file
done
