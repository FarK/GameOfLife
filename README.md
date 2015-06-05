Game Of Life With OpenMP and MPI
================================
This project is a research for the viability of an high-parallel implementation
of the Conway's 'game of life'. I've parallelized at thread level with OpenMP
and at process level with OpenMP.

Structure optimization
----------------------
I designed a complex structure in order to try to reduce the number of memory
accesses and improve the iteration time.

I've chosen a two dimensional array of pointers to node of linked list. In the
linked list I save the cells that must be checked. The array allow to access to
the neighbors with an constant order. On the other hand, the linked list allow
to check only the cells susceptible to change its state.

With this structure we obtain a processing time independent of world size, but
it dependent linearly on number of alive cells. If we have a number of alive
cells close to the world size, we should obtain a process time slightly higher
to an array based implementation approach.

![Structure scheme](doc/worldStructure.png?raw=true "World structure")

Thread parallelization
----------------------
For thread parallelization each thread processes an equal portion of the linked
list.

Process parallelization
-----------------------
For the process parallelization, i divide the world into equal portions along x
axis. Each process processes its portion, sends changes at limits to the top
and bottom process, and receives the changes at limits too.

Build and run Instructions
--------------------------
The top 'makefile' automatically creates 'build' directory, calls 'cmake' and
builds the project.

For run:
```
$> mpirun -n <number of process> build/gameOfLife <options (see help)>
```

Viewer script
-------------
For small worlds you can activate the 'record' flag (-r or --record) for
generate a record of whole execution. Later, you can view this record with the
'viewer.sh' script. The control keys are:

* 'p' : pause/continue
* '+' : increment velocity
* '-' : decrement velocity
* 'b' : backward
* < other key > : forward

Dependences
-----------
* openmpi v1.6.5
* openmp
* gnuplot [for generate graphics]

------

Results
-------
These tests were executed on a machine with an Intel Pentium Dual Core at 2 GHz
with 4GB of RAM.

You can view all results [here](doc/readme.md).
