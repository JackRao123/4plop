cmake_minimum_required(VERSION 3.16)
project(phevaluator)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SOURCES
    dptables.cpp
    evaluation_plo4.cpp
    hash.cpp
    tables_plo4.cpp
    card_sampler.cpp
    evaluator_plo4.cpp
    tables_bitwise.cpp
    hashtable.cpp
    kev/fast_eval.c
    kev/kev_eval.c
)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Add the main executable
add_executable(phevaluator_main main.cpp ${SOURCES})
