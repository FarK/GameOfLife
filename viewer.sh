#!/bin/bash

function setCursor() {
	echo -en "\033[$1;$2H"
}

nNodes=$#
nIterations=($1/*)
nIterations=${#nIterations[@]}
directories=($@)

for i in ${directories[@]}; do
	iterations+=($i/*)
done
nFiles=${#iterations[@]}

for i in $(seq 0 $((nIterations-1))); do
	for j in $(seq 0 $((nNodes-1))); do
		indx=$((i+j*nIterations))
		orderedIt+=(${iterations[$indx]})
	done
done

i=0
quit=0
period="0.5"
rperiod="-t$period"
pause="       "
clear
while [ $quit -eq 0 ]; do
	echo -e "${orderedIt[@]:$i:$nNodes}\tPeriod = $period    \t$pause\n"
	cat ${orderedIt[@]:$i:$nNodes}

	read -sr -n1 $rperiod key

	if [ "$key" = "b" ]; then
		(( i -= $nNodes*2 ))
		[ $i -lt "-$nNodes" ] && (( i = $nFiles - $nNodes*2 ))
	elif [ "$key" = "r" ]; then
		(( i = -$nNodes ))
	elif [ "$key" = "p" ]; then
		if [ $rperiod ]; then
			rperiod=
			pause="[PAUSE]"
		else
			rperiod="-t$period"
			pause="       "
		fi
	elif [ "$key" = "-" ]; then
		period=$(echo "$period + 0.1" | bc)
		rperiod="-t$period"
	elif [ "$key" = "+" ]; then
		period=$(echo "if($period > 0.1) $period - 0.1" else $period | bc)
		period=$(echo "if($period > 0.01) $period - 0.01" else $period | bc)
		rperiod="-t$period"
	elif [ "$key" = "q" ]; then
		quit=1
	fi

	(( i = (i + $nNodes)%$nFiles ))

	setCursor 0 0
done
clear
