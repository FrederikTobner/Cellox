cmake_minimum_required(VERSION 3.16)
# configure cellox config to show version
configure_file(cellox_config.h.in cellox_config.h)

set(HEADERS
    chunk_file.h
    chunk.h
    command_line_argument_parser.h
    compiler.h
    dynamic_value_array.h
    debug.h
    hash_table.h
    init.h
    lexer.h
    memory.h
    native_functions.h
    object.h
    string_utils.h
    value.h
    virtual_machine.h
)

set(SOURCES
    chunk_file.c 
    chunk.c
    command_line_argument_parser.c
    compiler.c
    compiler.h
    dynamic_value_array.c
    debug.c
    hash_table.c
    init.c 
    lexer.c
    main.c
    memory.c 
    native_functions.c
    object.c 
    string_utils.c
    value.c  
    virtual_machine.c
)
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Precompiles common.h to speed up compilation of the target
if(MSVC)    
    target_precompile_headers(${PROJECT_NAME} PUBLIC common.h common.c) # VisualStudio only accepts header files that also have a source file
else()
    target_precompile_headers(${PROJECT_NAME} PUBLIC common.h)
endif()

# Includes Libmath
if(UNIX)
    target_link_libraries(${PROJECT_NAME} m)
endif()

# for including the cellox_config.h file
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_BINARY_DIR}/src)

# Sets properties for the package created using cpack
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION "${Cellox_VERSION_MAJOR}.${Cellox_VERSION_MINOR}.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_NAME} Interpreter")
set(CPACK_BUILD_SOURCE_DIRS "${PROJECT_BINARY_DIR}/src")

include(CPack)
install(TARGETS ${PROJECT_NAME} DESTINATION bin)