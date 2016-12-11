cmake_minimum_required (VERSION 2.6) 
project (FallDetection) 

if(APPLE)
	set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")
	set (CMAKE_CXX_COMPILER "clang++")
	set (CMAKE_C_COMPILER "clang")
endif()


find_package(OpenCV 2.4 REQUIRED)

#find_package(libzmq REQUIRED)

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS} -Wno-sign-compare ")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_CXX_FLAGS}")

add_definitions(-std=c++11)

file(GLOB MYSOURCES	*.cpp *.c)
file(GLOB MYHEADERS	*.h *.hpp)

add_executable(FallDetection ${MYSOURCES} ${MYHEADERS} MessageTransmitter.cpp MessageTransmitter.h)

target_link_libraries(FallDetection 
	${OpenCV_LIBS}
		zmq
)
