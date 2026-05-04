if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    include("${CMAKE_CURRENT_LIST_DIR}/gcc.cmake")
elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    include("${CMAKE_CURRENT_LIST_DIR}/clang.cmake")
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    include("${CMAKE_CURRENT_LIST_DIR}/msvc.cmake")
elseif(CMAKE_C_COMPILER_ID STREQUAL "Intel")
    include("${CMAKE_CURRENT_LIST_DIR}/intel.cmake")
else()
    include("${CMAKE_CURRENT_LIST_DIR}/generic.cmake")
endif()

# Create the clx_toolchain INTERFACE library after CLX_COMPILER_IMPL_INCLUDE_DIR is set
add_subdirectory("${CMAKE_SOURCE_DIR}/src/conditionals/compiler" "${CMAKE_BINARY_DIR}/src/conditionals/compiler")