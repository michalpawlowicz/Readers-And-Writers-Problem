cmake_minimum_required(VERSION 3.6)
project(zad2)

set(CMAKE_C_STANDARD 11)

set(SOURCE_FILES main.c)
add_executable(zad2 ${SOURCE_FILES})
target_link_libraries(zad2 pthread)
target_link_libraries(zad2 g)
