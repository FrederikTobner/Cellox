set(LANGUAGE_BENCHMARKS ${PROJECT_NAME}Benchmarks)

set(BENCHMARK_HEADER_FILES
"benchmark_config.h"
"benchmark_runner.h"
"benchmark_argument_parser.h"
"common.h"
"dynamic_benchmark_config_array.h"
)

set(BENCHMARK_SOURCE_FILES
"benchmark_runner.c"
"benchmark_argument_parser.c"
"dynamic_benchmark_config_array.c"
"main.c"
)

# dependencies from the interpreter needed to build the benchmark runner tool
set(BENCHMARK_DEPENDENCIES_SOURCE_FILES
"${SOURCEPATH}/chunk_file.c"
"${SOURCEPATH}/chunk_optimizer.c"
"${SOURCEPATH}/chunk.c"
"${SOURCEPATH}/compiler.c"
"${SOURCEPATH}/dynamic_value_array.c"
"${SOURCEPATH}/debug.c"
"${SOURCEPATH}/garbage_collector.c"
"${SOURCEPATH}/hash_table.c"
"${SOURCEPATH}/initializer.c"
"${SOURCEPATH}/lexer.c"
"${SOURCEPATH}/memory_mutator.c"
"${SOURCEPATH}/native_functions.c"
"${SOURCEPATH}/object.c"
"${SOURCEPATH}/string_utils.c"
"${SOURCEPATH}/value.c"
"${SOURCEPATH}/virtual_machine.c"
)
set(BENCHMARK_DEPENDENCIES_HEADER_FILES
"${SOURCEPATH}/chunk_file.h"
"${SOURCEPATH}/chunk_optimizer.h"
"${SOURCEPATH}/chunk.h"
"${SOURCEPATH}/compiler.h"
"${SOURCEPATH}/dynamic_value_array.h"
"${SOURCEPATH}/debug.h"
"${SOURCEPATH}/garbage_collector.h"
"${SOURCEPATH}/hash_table.h"
"${SOURCEPATH}/initializer.h"
"${SOURCEPATH}/lexer.h"
"${SOURCEPATH}/memory_mutator.h"
"${SOURCEPATH}/native_functions.h"
"${SOURCEPATH}/object.h"
"${SOURCEPATH}/string_utils.h"
"${SOURCEPATH}/value.h"
"${SOURCEPATH}/virtual_machine.h"
)

add_compile_definitions(
# Directory of the cellox benchmarks
BENCHMARK_BASE_PATH="${PROJECT_SOURCE_DIR}/benchmark/benchmarks/"
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