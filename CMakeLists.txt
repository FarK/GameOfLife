cmake_minimum_required(VERSION 2.8)

project(gameOfLife)

set(CMAKE_C_COMPILER "mpicc")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -g --fast-math -Wall -fopenmp")

set(HDRS
	types.h
	utils.h
	world.h
	gol.h
	boundaryCells.h
	node.h
	)

set(SRCS
	world.c
	gol.c
	boundaryCells.c
	node.c
	)

add_executable(gameOfLife
	main.c
	${SRCS}
	${HDRS}
	)
