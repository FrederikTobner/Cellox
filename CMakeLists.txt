cmake_minimum_required(VERSION 3.16)
project(Cellox VERSION 0.1.0)

set(CMAKE_C_STANDARD 11)
#C++ 14 needed for Google Test Framework
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

include(CPack)

add_subdirectory(src)

enable_testing()

add_subdirectory(test)