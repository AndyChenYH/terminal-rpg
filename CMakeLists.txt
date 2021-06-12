cmake_minimum_required(VERSION 3.19)
project(terminal_rpg)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(terminal_rpg
        main.cpp
        main.h)
