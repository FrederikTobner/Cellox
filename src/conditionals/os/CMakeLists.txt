# OS abstraction layer
# Exactly one OS implementation is selected by CMake and exposed as clx_os.
#
# CLX_OS_SOURCES must be set by cmake/conditionals/os/*.cmake before this
# subdirectory is added.  It is a list of absolute paths to the .c files that
# make up the chosen platform layer.

if(NOT DEFINED CLX_OS_SOURCES OR CLX_OS_SOURCES STREQUAL "")
    message(FATAL_ERROR "CLX_OS_SOURCES must be set by cmake/conditionals/os/*.cmake before adding src/conditionals/os")
endif()

cellox_add_library(clx_os
    SOURCES
        ${CLX_OS_SOURCES}
    INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)