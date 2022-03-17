cmake_minimum_required(VERSION 3.16.3)
project(Splendor_ai)
# project(scratch)

set(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

include_directories(genetic_alg)
# include_directories(Scratches)

add_executable(Splendor_ai interface.cpp)
# add_executable(scratch Scratches/scratch.cpp)
