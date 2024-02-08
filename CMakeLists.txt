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

# Create executable
add_executable(chip8 ${SOURCES} ${HEADERS})

