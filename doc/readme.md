Results
=======

Suffix "*t2*" means two thread, "*p2*" two processes and no suffix means one
thread and one process.

Alive cells and world size VS time
----------------------------------
![AlivesCellsAndSize](cells_size.png "Alive cells and size")

As we can see the execution with multiple processes have worse result than an
serial execution. 

Iterations VS time
------------------
The algorithm scales linearly.

![Iterations](cells_size.png "Iterations")

Profiling
---------
The communication represent a significant proportion of the total iteration
time. And the parallelization with OpenMP have a slight cost too.

![1proc1thread](p1t1.png "1 proccess, 1 thread")
![1proc2thread](p1t2.png "1 proccess, 2 threads")
![2proc1thread](p2t1.png "2 proccesses, 1 thread")
