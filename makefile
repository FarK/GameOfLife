CMAKE_BUILD_DIR = build

all : ${CMAKE_BUILD_DIR} ${CMAKE_BUILD_DIR}/Makefile
	@ cd ${CMAKE_BUILD_DIR} && make

${CMAKE_BUILD_DIR}/Makefile : CMakeLists.txt
	@ cd ${CMAKE_BUILD_DIR} && cmake ../

${CMAKE_BUILD_DIR} :
	@ mkdir ${CMAKE_BUILD_DIR}

clean:
	@- rm -fr ${CMAKE_BUILD_DIR} 2>/dev/null
