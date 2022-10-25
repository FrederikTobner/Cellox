cmake_minimum_required(VERSION 3.16)
project(Cellox 
VERSION 0.1.0
)

# C99 standard is required to build the interpreter
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# Builds cellox interpreter
add_subdirectory(src)

enable_testing()

# Builds tests for the interpreter
add_subdirectory(test)