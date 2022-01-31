cmake_minimum_required(VERSION 3.20)
project(concurrent_file_search C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -lm")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "/Users/apple/Desktop/faks/SRV/concurrent_file_search/")

add_executable(main src/main.c src/helpers/helpers.h src/helpers/potpisi.h)
