#!/bin/bash

function setCursor() {
	echo -en "\033[$1;$2H"
}

nNodes=$#
nIterations=($1/*)
nIterations=${#nIterations[@]}
directories=($@)
iterations=(node?/*)
nFiles=${#iterations[@]}

for i in $(seq 0 $((nIterations-1))); do
	for j in $(seq 0 $((nNodes-1))); do
		indx=$((i+j*nIterations))
		orderedIt=(${orderedIt[@]} ${iterations[$indx]})
	done
done

clear
for i in $(seq 0 $nNodes $((nFiles-1))); do
	echo ${orderedIt[@]:$i:$nNodes}
	cat ${orderedIt[@]:$i:$nNodes}
	sleep 0.1
	setCursor 0 0
done
# clear
