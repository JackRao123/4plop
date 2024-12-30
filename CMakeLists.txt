cmake_minimum_required(VERSION 3.16)
project(4plop)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the submodule directory
add_subdirectory(hand_evaluator)

# Add the executable target
add_executable(4plop main.cpp)

# Link the hand_evaluator library to the executable
target_link_libraries(4plop PRIVATE hand_evaluator)