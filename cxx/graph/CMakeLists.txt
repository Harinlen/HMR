cmake_minimum_required(VERSION 3.0)

project(graph)

# Options
set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)

# Enable the lib includes.
include_directories(../libs/)

# Binaries
add_executable(graph
    src/main.cpp
    src/arguments.cpp
    ../libs/ui_utils.cpp
)

