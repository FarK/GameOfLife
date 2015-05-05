cmake_minimum_required(VERSION 2.8)

project(gameOfLife)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -g --fast-math -Wall")

set(HDRS
	world.h
	gol.h
	)

set(SRCS
	world.c
	gol.c
	)

add_executable(gameOfLife
	main.c
	${SRCS}
	${HDRS}
	)
