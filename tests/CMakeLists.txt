# Enable testing
enable_testing()

# Collect test sources
set(TEST_SOURCES
    # test sources
    node_test.cpp
    equity_calc_test.cpp
    profiling_test.cpp
    
    # implementation sources
    ../src/node.cpp
    ../src/chancenode.cpp
)

# Add the test executable
add_executable(tests ${TEST_SOURCES})

# Include the project's headers
target_include_directories(tests PRIVATE
    ${CMAKE_SOURCE_DIR}   # Include the root directory for headers
    ${CMAKE_SOURCE_DIR}/phevaluator # Include phevaluator headers
)

# Link Google Test and project libraries
target_link_libraries(tests PRIVATE gtest gtest_main phevaluator)

# Register tests with CTest
add_test(NAME NodeTest COMMAND tests)
