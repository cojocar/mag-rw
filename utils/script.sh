#!/bin/bash
for i in `seq -w $1 $2` 
do
	echo "processing test$i"
	python reader.py < test$i
done
