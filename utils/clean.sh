#!/bin/bash
for i in `seq -w $1 $2` 
do
	echo "deleting test$i"
	rm test$i
done
