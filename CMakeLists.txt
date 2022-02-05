cmake_minimum_required(VERSION 3.21)
project(Splendor_ai)
project(scratch)

set(CMAKE_CXX_STANDARD 17)
include_directories(genetic_alg)
include_directories(Scratches)

add_executable(Splendor_ai interface.cpp)
add_executable(scratch Scratches/scratch.cpp)
