cmake_minimum_required(VERSION 3.21)
project(Splendor_ai)

set(CMAKE_CXX_STANDARD 17)
include_directories(genetic_alg)

add_executable(Splendor_ai genetic_alg/GeneticAlgorithm.cpp)
