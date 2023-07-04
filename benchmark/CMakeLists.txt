set(LANGUAGE_BENCHMARKS ${PROJECT_NAME}Benchmarks)

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

# The source files of the benchmark runner tool
set(BENCHMARK_HEADER_FILES
"benchmark_config.h"
"benchmark_runner.h"
"benchmark_argument_parser.h"
"common.h"
"dynamic_benchmark_config_array.h"
)

# The header files of the benchmark runner tool
set(BENCHMARK_SOURCE_FILES
"benchmark_runner.c"
"benchmark_argument_parser.c"
"dynamic_benchmark_config_array.c"
"main.c"
)

# The source files from the cellox compiler needed to build the benchmark runner tool
set(BENCHMARK_DEPENDENCIES_SOURCE_FILES
"${SOURCEPATH}/initializer.c"
"${SOURCEPATH}/string_utils.c"
"${SOURCEPATH}/backend/garbage_collector.c"
"${SOURCEPATH}/backend/memory_mutator.c"
"${SOURCEPATH}/backend/native_functions.c"
"${SOURCEPATH}/backend/virtual_machine.c"
"${SOURCEPATH}/byte-code/chunk.c"
"${SOURCEPATH}/byte-code/chunk_disassembler.c"
"${SOURCEPATH}/byte-code/chunk_file.c"
"${SOURCEPATH}/frontend/compiler.c"
"${SOURCEPATH}/frontend/lexer.c"
"${SOURCEPATH}/language-models/object.c"
"${SOURCEPATH}/language-models/value.c"
"${SOURCEPATH}/language-models/data-structures/dynamic_value_array.c"
"${SOURCEPATH}/language-models/data-structures/value_hash_table.c"
"${SOURCEPATH}/middle-end/chunk_optimizer.c"
)
# The header files of the cellox compiler that are needed to build the benchmark runner tool
set(BENCHMARK_DEPENDENCIES_HEADER_FILES
"${SOURCEPATH}/initializer.h"
"${SOURCEPATH}/string_utils.h"
"${SOURCEPATH}/backend/garbage_collector.h"
"${SOURCEPATH}/backend/memory_mutator.h"
"${SOURCEPATH}/backend/native_functions.h"
"${SOURCEPATH}/backend/virtual_machine.h"
"${SOURCEPATH}/byte-code/chunk.h"
"${SOURCEPATH}/byte-code/chunk_disassembler.h"
"${SOURCEPATH}/byte-code/chunk_file.h"
"${SOURCEPATH}/frontend/compiler.h"
"${SOURCEPATH}/frontend/lexer.h"
"${SOURCEPATH}/language-models/object.h"
"${SOURCEPATH}/language-models/value.h"
"${SOURCEPATH}/language-models/data-structures/dynamic_value_array.h"
"${SOURCEPATH}/language-models/data-structures/value_hash_table.h"
"${SOURCEPATH}/middle-end/chunk_optimizer.h"
)

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
${BENCHMARK_DEPENDENCIES_SOURCE_FILES} ${BENCHMARK_DEPENDENCIES_HEADER_FILES} 
${BENCHMARK_SOURCE_FILES} ${BENCHMARK_HEADER_FILES})

# Includes Libmath under unix-like operating systems
if(UNIX)
    target_link_libraries(${LANGUAGE_BENCHMARKS} m)
endif()

target_include_directories(${LANGUAGE_BENCHMARKS} PUBLIC ${PROJECT_BINARY_DIR}/src)