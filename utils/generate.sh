#!/bin/bash
for i in `seq -w $1 $2` 
do
	echo "generating test$i"
	echo "$i" > test$i
	echo "1" >> test$i
	cat $3 >> test$i
done
