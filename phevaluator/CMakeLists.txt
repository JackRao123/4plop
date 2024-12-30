cmake_minimum_required(VERSION 3.16)
project(phevaluator)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SOURCES
    tables/dptables.cpp
    tables/hashtable.cpp
    tables/tables_bitwise.cpp
    tables/tables_plo4.cpp
    tables/7462.cpp

    src/rank.cpp
    src/card_sampler.cpp
    src/evaluator_plo4.cpp
    src/hash.cpp
)

# Define the phevaluator library
add_library(phevaluator STATIC ${SOURCES})

# Include directories for the library
target_include_directories(phevaluator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Include directories
# include_directories(${CMAKE_CURRENT_SOURCE_DIR})


# Add the main executable
# add_executable(phevaluator_main main.cpp ${SOURCES})
