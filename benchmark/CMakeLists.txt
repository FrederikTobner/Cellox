# deletes build folder to ensure there are no makefiles that make the build fail (multiple target patterns)
if(MSVC)
execute_process(COMMAND ${CMAKE_COMMAND} "-E" "remove_directory" ${PROJECT_BINARY_DIR}/benchmark)
endif()

set(THIS ${PROJECT_NAME}Benchmarks)

set(BENCHMARK_HEADERS
benchmark.h
)

set(BENCHMARK_SOURCES
benchmark.c 
main.c
)

add_compile_definitions(
BENCHMARKS_RUNNING
# Directory of the cellox benchmarks
BENCHMARK_BASE_PATH="${PROJECT_SOURCE_DIR}/benchmark/benchmarks/"
)

# Precompiles common.h to speed up compilation of the target
if(MSVC)    
    target_precompile_headers(${PROJECT_NAME} PUBLIC ${SOURCEPATH}common.h ${SOURCEPATH}common.c) # VisualStudio only accepts header files that also have a source file
else()
    target_precompile_headers(${PROJECT_NAME} PUBLIC ${SOURCEPATH}common.h)
endif()


add_executable(${THIS} ${CELLOXSOURCES} ${CELLOXHEADERS} ${BENCHMARK_HEADERS} ${BENCHMARK_SOURCES})

# Includes SourcePath of the compiler for shorter includes
target_include_directories(${THIS} PUBLIC ${SOURCEPATH})