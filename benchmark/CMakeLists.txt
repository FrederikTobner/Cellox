set(LANGUAGE_BENCHMARKS ${PROJECT_NAME}Benchmarks)

cellox_add_compiler_compile_definitions()
cellox_add_os_compile_definitions()

# Checking the dependencies of the benchmark runner tool
if(UNIX)
    # Check dependecies under unix-like systems
    CHECK_INCLUDE_FILE("dirent.h" DIRENT_AVAILABLE)
    if(NOT ${DIRENT_AVAILABLE})
        message(FATAL_ERROR "dirent.h is required to build the benchmark runner under unix-like systems. \
\   \   Please make sure it is available to the compiler and try again after that")
    endif() # dirent.h not available
elseif(WIN32)
    # Check dependecies under windows
    CHECK_INCLUDE_FILE("fileapi.h" FILEAPI_AVAILABLE)
    CHECK_INCLUDE_FILE("windows.h" WINDOWS_AVAILABLE)
    if(NOT ${FILEAPI_AVAILABLE})
        message(FATAL_ERROR "conio.h is required to build the compiler under windows. \
\   \   Please make sure it is available to the compiler and try again after that")
    endif() # conio.h not available
    if(NOT ${WINDOWS_AVAILABLE})
        message(FATAL_ERROR "windows.h is required to build the compiler under windows. \
\   \   Please make sure it is available to the compiler and try again after that")
    endif() # windows.h not available
endif()

# Header files of the benchmark runner tool
file(GLOB_RECURSE BENCHMARK_HEADER_FILES *.h)
# Source files of the benchmark runner tool
file(GLOB_RECURSE BENCHMARK_SOURCE_FILES *.c)

add_compile_definitions(
# Directory of the cellox benchmarks
BENCHMARK_BASE_PATH="${PROJECT_SOURCE_DIR}/benchmark/benchmarks/"
# Directory of the cellox benchmark results
RESULTS_BASE_PATH="${PROJECT_SOURCE_DIR}/benchmark/results"
)

if(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]")
    add_compile_definitions(BUILD_DEBUG)
endif()

add_executable(${LANGUAGE_BENCHMARKS} 
${CELLOX_SOURCE_FILES} ${CELLOX_HEADER_FILES} 
${BENCHMARK_SOURCE_FILES} ${BENCHMARK_HEADER_FILES})

# Includes Libmath under unix-like operating systems
if(UNIX)
    target_link_libraries(${LANGUAGE_BENCHMARKS} m)
endif()

target_include_directories(${LANGUAGE_BENCHMARKS} PUBLIC ${PROJECT_BINARY_DIR}/src)