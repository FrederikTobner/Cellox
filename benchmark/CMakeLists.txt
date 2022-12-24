set(LANGUAGE_BENCHMARKS ${PROJECT_NAME}Benchmarks)

set(BENCHMARK_HEADERS
"benchmark_config.h"
"benchmark_runner.h"
"benchmark_argument_parser.h"
"common.h"
"dynamic_benchmark_config_array.h"
)

set(BENCHMARK_SOURCES
"benchmark_runner.c"
"benchmark_argument_parser.c"
"dynamic_benchmark_config_array.c"
"main.c"
)

add_compile_definitions(
BENCHMARKS_RUNNING
# Directory of the cellox benchmarks
BENCHMARK_BASE_PATH="${PROJECT_SOURCE_DIR}/benchmark/benchmarks/"
)

if(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]")
    add_compile_definitions(BUILD_DEBUG)
endif()

add_executable(${LANGUAGE_BENCHMARKS} ${CELLOXSOURCES} ${CELLOXHEADERS} ${BENCHMARK_HEADERS} ${BENCHMARK_SOURCES})

# Includes Libmath
if(UNIX)
    target_link_libraries(${LANGUAGE_BENCHMARKS} m)
endif()

target_include_directories(${LANGUAGE_BENCHMARKS} PUBLIC ${PROJECT_BINARY_DIR}/src)