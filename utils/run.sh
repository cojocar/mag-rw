#!/bin/bash

if [ $# -lt 4 ]
then
	echo "Correct Syntax is: ./run.sh lower_limit upper_limit input_file out_putfile"
else
	./generate.sh $1 $2 $3
	./script.sh $1 $2 > $4
	./clean.sh $1 $2
	cat $4
fi

