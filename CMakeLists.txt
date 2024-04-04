cmake_minimum_required(VERSION 3.27)
project(Pikachu_Main_Menu)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable(Pikachu_Main_Menu main.cpp
        main.h)
