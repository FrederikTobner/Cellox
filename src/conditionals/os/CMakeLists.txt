# OS abstraction layer
# Exactly one OS implementation is selected by CMake and exposed as clx_os.

if(NOT DEFINED CLX_OS_LAYER_DIR OR CLX_OS_LAYER_DIR STREQUAL "")
    message(FATAL_ERROR "CLX_OS_LAYER_DIR must be set by cmake/conditionals/os/*.cmake before adding src/conditionals/os")
endif()

cellox_add_library(clx_os
    SOURCES
        ${CLX_OS_LAYER_DIR}/fs.c
        ${CLX_OS_LAYER_DIR}/path.c
        ${CLX_OS_LAYER_DIR}/platform.c
        ${CLX_OS_LAYER_DIR}/stdio.c
        ${CLX_OS_LAYER_DIR}/temp.c
        ${CLX_OS_LAYER_DIR}/time.c
    INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)