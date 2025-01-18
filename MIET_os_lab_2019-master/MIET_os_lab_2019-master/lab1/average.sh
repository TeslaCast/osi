#!/bin/bash

sum=0

if [ $# -eq 0 ]
then
	count=0
	while read LINE 
	do 
 		count=$[count + 1]
 		sum=$[ sum + $LINE ]
	done 
	midle=$[ sum / $count ]
	echo $count
	echo $midle
else
	for params in $@
	do
 		sum=$[ sum + $params ]
	done
	midle=$[ sum / $# ]
	echo $#
	echo $midle
fi

