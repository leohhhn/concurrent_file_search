cmake_minimum_required(VERSION 3.20)
project(concurrent_file_search C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -lm")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "/home/lav/Desktop/faks2122/SRV/concurrent_file_search")

add_executable(main src/main.c src/threads.h src/helpers/declarations.h src/helpers/generalHelpers.h src/helpers/threadHelpers.h src/blockingArray.h)
target_link_libraries(main m)