cmake_minimum_required(VERSION 3.14)
project(chip8)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)

# Add source files
set(SOURCES
    source/chip.cpp
    source/main.cpp
    source/platform.cpp
)

# Add header files
set(HEADERS
    source/chip.hpp
    source/platform.hpp
)

# Include the glad source directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/source/glad)

# Create executable
add_executable(chip8 ${SOURCES} ${HEADERS} source/glad/glad.c)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Find SDL2
find_package(SDL2 REQUIRED)

# Link libraries
target_link_libraries(chip8 ${OPENGL_LIBRARIES} ${SDL2_LIBRARIES})

