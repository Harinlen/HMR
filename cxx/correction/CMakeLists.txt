cmake_minimum_required(VERSION 3.0)

project(correction)

# Options
set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)

# Enable the lib includes.
include_directories(../libs/)

# Binaries
add_executable(correction
    ../libs/hp_bam_parser.cpp
    ../libs/hp_bgzf_parser.cpp
    ../libs/hp_bgzf_queue.cpp
    ../libs/hp_file_map.cpp
    ../libs/hp_unzip.cpp
    ../libs/ui_utils.cpp
    src/arguments.cpp
    src/bam_correction.cpp
    src/hp_fasta_parser.cpp
    src/main.cpp
)
target_link_libraries(correction pthread deflate)
