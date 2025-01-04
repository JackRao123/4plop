cmake_minimum_required(VERSION 3.16)
project(4plop)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the phevaluator library
add_subdirectory(phevaluator)

set(SOURCES 

    main.cpp 

    cfr.h
    deck.h
    helper.h
    equity_calc.h
    player.h

)


# Add the main executable
add_executable(4plop ${SOURCES})




# Link the phevaluator library to the executable
target_link_libraries(4plop PRIVATE phevaluator)


# Set the default startup project for Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT 4plop)
