#!/bin/bash

HEADER="ITERATIONS\tSIZE\tCELLS\tTOTAL\tMPI_IT\tCOMM\tOMP_IT\tCELL_CHK\tWORLD_UP\
\tTHREAD_0\tTHREAD_1\tTHREAD_2\tTHREAD_3\tTHREAD_4\tTHREAD_5\tTHREAD_6\tTHREAD_7\tTHREAD_8"

ITERATIONS=5000
CELLS=5000
SIZE=1000
EXECUTABLE="build/gameOfLife"

# run proccess threads size cells iterations
function run() {
	mpirun -n $1 $EXECUTABLE -t$2 -s$3 -c$4 -i$5
}

function sizeRun() {
	run 1 1 ${1}x${1} $CELLS $ITERATIONS
}

function sizeRun_t2() {
	run 1 2 ${1}x${1} $CELLS $ITERATIONS
}

function sizeRun_n2() {
	run 2 1 ${1}x${1} $CELLS $ITERATIONS
}

function cellsRun() {
	run 1 1 ${SIZE}x${SIZE} $1 $ITERATIONS
}

function cellsRun_t2() {
	run 1 2 ${SIZE}x${SIZE} $1 $ITERATIONS
}

function cellsRun_n2() {
	run 2 1 ${SIZE}x${SIZE} $1 $ITERATIONS
}

function iterationsRun() {
	run 1 1 ${SIZE}x${SIZE} $CELLS $1
}

function iterationsRun_t2() {
	run 1 2 ${SIZE}x${SIZE} $CELLS $1
}

function iterationsRun_n2() {
	run 2 1 ${SIZE}x${SIZE} $CELLS $1
}

#addHeader filein fileout
function addHeader() {
	echo -e $HEADER | cat - $1 > "${2}"
}

#test init points final xRun
function test() {
	init=$1
	points=$2
	final=$3
	xRun=$4

	inc=$(( ($final - $init)/$points ))
	perc=0

	echo -en "$xRun [$init, $final] - $points: "
	echo -en "\033[s"
	echo -en "  0%\n"

	for ((i=0, param=$init; i<=$points; i++, param+=$inc))
	do
		$xRun $param

		perc=$(( i*100/points ))
		echo -en "\033[u"
		echo -en "${perc}%   \n"
	done

}


###########
## TESTS ##
###########

> stats.data
test 9000 30 20000 cellsRun
addHeader stats.data gnuplot/test.data

> stats.data
test 1 100 10000 cellsRun
addHeader stats.data gnuplot/cells.data

> stats.data
test 1000 50 10000 sizeRun
addHeader stats.data gnuplot/size.data

> stats.data
test 1 100 10000 iterationsRun
addHeader stats.data gnuplot/iterations.data

> stats.data
test 1 100 10000 cellsRun_t2
addHeader stats.data gnuplot/cells_t2.data

> stats.data
test 1000 50 10000 sizeRun_t2
addHeader stats.data gnuplot/size_t2.data

> stats.data
test 1 100 10000 iterationsRun_t2
addHeader stats.data gnuplot/iterations_t2.data

> stats.data
test 1 100 10000 cellsRun_n2
addHeader stats.data gnuplot/cells_n2.data

> stats.data
test 1000 50 10000 sizeRun_n2
addHeader stats.data gnuplot/size_n2.data

> stats.data
test 1 100 10000 iterationsRun_n2
addHeader stats.data gnuplot/iterations_n2.data

> stats.data
run 1 1 ${SIZE}x${SIZE} $CELLS $ITERATIONS
addHeader stats.data gnuplot/n1t1.data

> stats.data
run 1 2 ${SIZE}x${SIZE} $CELLS $ITERATIONS
addHeader stats.data gnuplot/n1t2.data

> stats.data
run 2 1 ${SIZE}x${SIZE} $CELLS $ITERATIONS
addHeader stats.data gnuplot/n2t1.data
