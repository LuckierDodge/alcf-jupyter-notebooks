#!/bin/bash


for file in /gpfs/mira-fs0/logs/darshan/mira/201[5]/[1-12]/[1-31]/*.darshan.gz
do
	echo $file >> list.txt
done

# for file in /gpfs/mira-fs0/logs/darshan/mira/201[5-9]/[1-12]/[1-31]/*.darshan.gz
# do
# 	echo $file >> list.txt
# done
