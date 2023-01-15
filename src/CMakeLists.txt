# configure cellox config to show version
configure_file(cellox_config.h.in cellox_config.h)

# source files of the interpreter
set(CELLOX_SOURCE_FILES
"${SOURCEPATH}/chunk_file.c"
"${SOURCEPATH}/chunk_optimizer.c"
"${SOURCEPATH}/chunk.c"
"${SOURCEPATH}/command_line_argument_parser.c"
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

# Headers of the interpreter
set(CELLOX_HEADER_FILES
"${SOURCEPATH}/chunk_file.h"
"${SOURCEPATH}/chunk_optimizer.h"
"${SOURCEPATH}/chunk.h"
"${SOURCEPATH}/command_line_argument_parser.h"
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

add_executable(${PROJECT_NAME} ${CELLOX_SOURCE_FILES} ${CELLOX_HEADER_FILES} main.c)

# Precompiles common.h to speed up compilation of the target
if(MSVC)
    # VisualStudio only accepts header files that also have a source file    
    target_precompile_headers(${PROJECT_NAME} PUBLIC common.h common.c) 
else()
    target_precompile_headers(${PROJECT_NAME} PUBLIC common.h)
endif()

# Includes Libmath under unix-like systems
if(UNIX)
    target_link_libraries(${PROJECT_NAME} m)
endif()

if(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]")
    if(DEBUG_PRINT_BYTECODE)
        add_compile_definitions(DEBUG_PRINT_CODE)
    endif()
    if(DEBUG_TRACE_EXECUTION)
        add_compile_definitions(DEBUG_TRACE_EXECUTION)
    endif()
    if(DEBUG_STRESS_GARBAGE_COLLECTOR)
        add_compile_definitions(DEBUG_STRESS_GC)
    endif()
    if(DEBUG_LOG_GARBAGE_COLLECTOIN)
        add_compile_definitions(DEBUG_LOG_GC)
    endif()
    add_compile_definitions(BUILD_DEBUG)
endif()

# for including the cellox_config.h file
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_BINARY_DIR}/src)

if(NOT CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]")
    # Sets properties for the package created using cpack
    set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
    set(CPACK_PACKAGE_VERSION "${Cellox_VERSION_MAJOR}.${Cellox_VERSION_MINOR}.${Cellox_VERSION_PATCH}")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_NAME} Interpreter")
    set(CPACK_BUILD_SOURCE_DIRS "${PROJECT_BINARY_DIR}/src")
    set(CPACK_PACKAGE_VENDOR ${PROJECT_VENDOR})
    
    # Sets properties for NSIS (nullsoft scriptable install system - https://sourceforge.net/projects/nsis/)
    set(CPACK_NSIS_MUI_ICON "${PROJECT_DIRECTORY}/icons/file_type_cellox.ico")
    set(CPACK_NSIS_MODIFY_PATH "ON")
    set(CPACK_NSIS_URL_INFO_ABOUT ${PROJECT_PAGE})
    set(CPACK_NSIS_HELP_LINK ${PROJECT_HELP_PAGE})
    
    include(CPack)
    install(TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()
