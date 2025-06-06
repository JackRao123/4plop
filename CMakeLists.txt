cmake_minimum_required(VERSION 3.16)
project(4plop)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

 
# Add the phevaluator library
add_subdirectory(phevaluator)

# Add the main application sources
set(SOURCES 
    src/main.cpp 
    src/simulation.h
    src/node.h
    src/chancenode.h
    src/deck.h
    src/helper.h
    src/equity_calc.h
    src/player.h
    src/gui.h
    src/gamestate.h
    src/node.cpp
    src/chancenode.cpp
)

# Add the main executable
add_executable(4plop ${SOURCES})

# Link the phevaluator library to the executable
target_link_libraries(4plop PRIVATE phevaluator)

# --- GLFW+IMGUI --- 
add_subdirectory(glfw)
# add_subdirectory(imgui)

# imgui repo doesn’t have its own CMake by default, so we’ll drop one in:
add_library(imgui STATIC
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_tables.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.cpp

    # #for easier debugging in Visual studio
    # idk how to add these 
    # misc/debuggers/imgui.natvis
    # misc/debuggers/imgui.natstepfilter
)
target_include_directories(imgui PUBLIC
    imgui
    imgui/backends
)
target_link_libraries(imgui PUBLIC
    glfw
    opengl32 # i need this? windows OS provides this apparently?
)
target_link_libraries(4plop PRIVATE imgui)




# --- Google Test Integration --- 
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# Ensure Google Mock is not built
set(BUILD_GMOCK OFF CACHE BOOL "Disable building Google Mock")
# Add GoogleTest as a subdirectory
add_subdirectory(googletest)
 
# Add the tests directory
add_subdirectory(tests)

# Set the default startup project for Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT 4plop)
