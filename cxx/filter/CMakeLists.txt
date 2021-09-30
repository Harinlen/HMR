cmake_minimum_required(VERSION 3.0)

project(filter)

# Options
set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)

# Enable the lib includes.
include_directories(../libs/)

# Binaries
add_executable(filter
    src/main.cpp
    src/arguments.cpp
    src/bam_filter.cpp
    src/compose_reduced.cpp
    src/enzyme.cpp
    src/hp_fasta_search.cpp
    ../libs/hp_unzip.cpp
    ../libs/hp_zip.cpp
    ../libs/hp_zip_crc32.cpp
    ../libs/ui_utils.cpp
    ../libs/hp_bam_parser.cpp
    ../libs/hp_bgzf_parser.cpp
    ../libs/hp_bgzf_queue.cpp
    ../libs/hp_file_map.cpp
)
target_link_libraries(filter pthread deflate)
