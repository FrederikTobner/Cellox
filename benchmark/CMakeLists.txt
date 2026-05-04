set(LANGUAGE_BENCHMARKS ${PROJECT_NAME}Benchmarks)

clx_os_check_benchmark_headers()

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

cellox_add_executable(${LANGUAGE_BENCHMARKS}
    SOURCES
        ${BENCHMARK_SOURCE_FILES}
    DEPENDENCIES
        clx_driver
)