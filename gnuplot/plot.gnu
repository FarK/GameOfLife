set terminal pngcairo size 960,720 enhanced
set title\
"DEFAULT:\n\t\
Initial alive cells = 5000\n\
\t\tSize of World = 1000x1000\n\
Iterations = 5000"

## FITTING ####
fitCells(x) = m1*x + b1
fitSize(x) = m2*x + b2
fitIters(x) = m3*x + b3

fitCells_t(x) = mt1*x + bt1
fitSize_t(x) = mt2*x + bt2
fitIters_t(x) = mt3*x + bt3

fitCells_n(x) = mn1*x + bn1
fitSize_n(x) = mn2*x + bn2
fitIters_n(x) = mn3*x + bn3

m1 = mn1 = mt1 = 0.01
b1 = bn1 = bt1 = 6000
m2 = mn2 = mt2 = 0.01
b2 = 1
bn2 = 1.4
bt2 = 0.8

fit [2000:10000] fitCells(x) 'cells.data' using "CELLS":"TOTAL" via m1, b1
fit [0:1e8] fitSize(x) 'size.data' using "SIZE":"TOTAL" via m2, b2
fit [1:10000] fitIters(x) 'iterations.data' using "ITERATIONS":"TOTAL" via m3, b3

fit [2000:10000] fitCells_t(x) 'cells_t2.data' using "CELLS":"TOTAL" via mt1, bt1
fit [0:1e8] fitSize_t(x) 'size_t2.data' using "SIZE":"TOTAL" via mt2, bt2
fit [1:10000] fitIters_t(x) 'iterations_t2.data' using "ITERATIONS":"TOTAL" via mt3, bt3

fit [2000:10000] fitCells_n(x) 'cells_n2.data' using "CELLS":"TOTAL" via mn1, bn1
fit [0:1e8] fitSize_n(x) 'size_n2.data' using "SIZE":"TOTAL" via mn2, bn2
fit [1:10000] fitIters_n(x) 'iterations_n2.data' using "ITERATIONS":"TOTAL" via mn3, bn3


## PLOT SIZE AND CELLS #####
set yrange[0:4]
set grid y
set key left top
set ylabel "time (s)"
set xlabel "alive cells"
set x2label "world size"
set x2tics out

set pointsize 0.3

set output 'cells_size.png'

plot 'cells.data' using "CELLS":"TOTAL" title 'cells' axes x1y1 lc 1 pt 1,\
     'size.data'  using "SIZE":"TOTAL" title 'size' axes x2y1 lc 2 pt 1,\
     'cells_t2.data' using "CELLS":"TOTAL" title 'cells_{t2}' axes x1y1 lc 3 pt 7,\
     'size_t2.data'  using "SIZE":"TOTAL" title 'size_{t2}' axes x2y1 lc 4 pt 7,\
     'cells_n2.data' using "CELLS":"TOTAL" title 'cells_{p2}' axes x1y1 lc 7 pt 7,\
     'size_n2.data'  using "SIZE":"TOTAL" title 'size_{p2}' axes x2y1 lc 6 pt 7,\
     fitCells(x) title '' lc 1,\
     fitSize(x) title '' axes x2y1 lc 2,\
     fitCells_t(x) title '' lc 3,\
     fitSize_t(x) title '' axes x2y1 lc 4,\
     fitCells_n(x) title '' lc 7,\
     fitSize_n(x) title '' axes x2y1 lc 6


## PLOT ITERATIONS ####
# set term wxt 2 #This plot in new window
set output 'iterations.png'
set xlabel "iterations"
unset x2tics
unset x2label

plot 'iterations.data' using "ITERATIONS":"TOTAL" title 't1 p1' lc 1 pt 7,\
     'iterations_t2.data' using "ITERATIONS":"TOTAL" title 't2 p1' lc 2 pt 7,\
     'iterations_n2.data' using "ITERATIONS":"TOTAL" title 't1 p2' lc 3 pt 7,\
     fitIters(x) title '' lc 1,\
     fitIters_t(x) title '' lc 2,\
     fitIters_n(x) title '' lc 3

## PLOT BARS ####
set autoscale
set xrange[-0.5:2.5]
set key right top
set xlabel "iterations"
set ylabel "time (ms)"
unset xlabel
unset xtics

set style data histogram
set style histogram rowstacked
set style fill solid
set boxwidth 0.3 absolute

set output 'p1t1.png'
plot 'n1t1.data' using (column("MPI_IT")*1000) title 'Iteration MPI',\
	newhistogram at 0.3,\
	'' using (column('OMP_IT')*1000) title 'Iteration OMP',\
	'' using (column('COMM')*1000) title 'Communications',\
	newhistogram at 0.6,\
	'' using (column('CELL_CHK')*1000) title 'Cell Checking',\
	'' using (column('WORLD_UP')*1000) title 'World update',\
	newhistogram at 0.9,\
	'' using (column('THREAD_0')*1000) title 'Thread 0'

set output 'p1t2.png'
plot 'n1t2.data' using (column("MPI_IT")*1000) title 'Iteration MPI',\
	newhistogram at 0.3,\
	'' using (column('OMP_IT')*1000) title 'Iteration OMP',\
	'' using (column('COMM')*1000) title 'Communications',\
	newhistogram at 0.6,\
	'' using (column('CELL_CHK')*1000) title 'Cell Checking',\
	'' using (column('WORLD_UP')*1000) title 'World update',\
	newhistogram at 0.9,\
	'' using (column('THREAD_0')*1000) title 'Thread 0',\
	newhistogram at 1.2,\
	'' using (column('THREAD_1')*1000) title 'Thread 1'

set output 'p2t1.png'
plot 'n2t1.data' using (column("MPI_IT")*1000) title 'Iteration MPI',\
	newhistogram at 0.3,\
	'' using (column('OMP_IT')*1000) title 'Iteration OMP',\
	'' using (column('COMM')*1000) title 'Communications',\
	newhistogram at 0.6,\
	'' using (column('CELL_CHK')*1000) title 'Cell Checking',\
	'' using (column('WORLD_UP')*1000) title 'World update',\
	newhistogram at 0.9,\
	'' using (column('THREAD_0')*1000) title 'Thread 0'

pause -1
