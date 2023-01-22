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
"${SOURCEPATH}/backend/chunk_file.c"
"${SOURCEPATH}/middleend/chunk_optimizer.c"
"${SOURCEPATH}/frontend/chunk.c"
"${SOURCEPATH}/frontend/compiler.c"
"${SOURCEPATH}/backend/dynamic_value_array.c"
"${SOURCEPATH}/backend/debug.c"
"${SOURCEPATH}/backend/garbage_collector.c"
"${SOURCEPATH}/backend/hash_table.c"
"${SOURCEPATH}/initializer.c"
"${SOURCEPATH}/frontend/lexer.c"
"${SOURCEPATH}/backend/memory_mutator.c"
"${SOURCEPATH}/backend/native_functions.c"
"${SOURCEPATH}/backend/object.c"
"${SOURCEPATH}/string_utils.c"
"${SOURCEPATH}/backend/value.c"
"${SOURCEPATH}/backend/virtual_machine.c"
)
set(BENCHMARK_DEPENDENCIES_HEADER_FILES
"${SOURCEPATH}/backend/chunk_file.h"
"${SOURCEPATH}/middleend/chunk_optimizer.h"
"${SOURCEPATH}/frontend/chunk.h"
"${SOURCEPATH}/frontend/compiler.h"
"${SOURCEPATH}/backend/dynamic_value_array.h"
"${SOURCEPATH}/backend/debug.h"
"${SOURCEPATH}/backend/garbage_collector.h"
"${SOURCEPATH}/backend/hash_table.h"
"${SOURCEPATH}/initializer.h"
"${SOURCEPATH}/frontend/lexer.h"
"${SOURCEPATH}/backend/memory_mutator.h"
"${SOURCEPATH}/backend/native_functions.h"
"${SOURCEPATH}/backend/object.h"
"${SOURCEPATH}/string_utils.h"
"${SOURCEPATH}/backend/value.h"
"${SOURCEPATH}/backend/virtual_machine.h"
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