# configure cellox config to show version
configure_file(cellox_config.h.in cellox_config.h)

if(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]")
    # source files of the compiler if the build-type is debug
    set(CELLOX_SOURCE_FILES
    "${SOURCEPATH}/command_line_argument_parser.c"
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

    # Headers of the  compiler if the build-type is debug
    set(CELLOX_HEADER_FILES
    "${SOURCEPATH}/command_line_argument_parser.h"
    "${SOURCEPATH}/initializer.h"
    "${SOURCEPATH}/string_utils.h"
    "${SOURCEPATH}/backend/garbage_collector.h"
    "${SOURCEPATH}/backend/memory_mutator.h"
    "${SOURCEPATH}/backend/native_functions.h"
    "${SOURCEPATH}/backend/virtual_machine.h"
    "${SOURCEPATH}/byte-code/chunk.h"
    "${SOURCEPATH}/byte-code/chunk_file.h"
    "${SOURCEPATH}/byte-code/chunk_disassembler.h"
    "${SOURCEPATH}/frontend/compiler.h"
    "${SOURCEPATH}/frontend/lexer.h"
    "${SOURCEPATH}/middle-end/chunk_optimizer.h"
    "${SOURCEPATH}/language-models/object.h"
    "${SOURCEPATH}/language-models/value.h"
    "${SOURCEPATH}/language-models/data-structures/dynamic_value_array.h"
    "${SOURCEPATH}/language-models/data-structures/value_hash_table.h"
    )
else()
    # source files of the compiler if the build-type is not debug
    set(CELLOX_SOURCE_FILES
    "${SOURCEPATH}/command_line_argument_parser.c"
    "${SOURCEPATH}/initializer.c"
    "${SOURCEPATH}/string_utils.c"
    "${SOURCEPATH}/backend/garbage_collector.c"
    "${SOURCEPATH}/backend/memory_mutator.c"
    "${SOURCEPATH}/backend/native_functions.c"
    "${SOURCEPATH}/backend/virtual_machine.c"
    "${SOURCEPATH}/byte-code/chunk.c"
    "${SOURCEPATH}/byte-code/chunk_file.c"
    "${SOURCEPATH}/frontend/compiler.c"
    "${SOURCEPATH}/frontend/lexer.c"
    "${SOURCEPATH}/language-models/object.c"
    "${SOURCEPATH}/language-models/value.c"
    "${SOURCEPATH}/language-models/data-structures/dynamic_value_array.c"
    "${SOURCEPATH}/language-models/data-structures/value_hash_table.c"
    "${SOURCEPATH}/middle-end/chunk_optimizer.c"
    )
    
    # Headers of the compiler if the build-type is not debug
    set(CELLOX_HEADER_FILES
    "${SOURCEPATH}/command_line_argument_parser.h"
    "${SOURCEPATH}/initializer.h"
    "${SOURCEPATH}/string_utils.h"
    "${SOURCEPATH}/backend/garbage_collector.h"
    "${SOURCEPATH}/backend/memory_mutator.h"
    "${SOURCEPATH}/backend/native_functions.h"
    "${SOURCEPATH}/backend/virtual_machine.h"
    "${SOURCEPATH}/byte-code/chunk.h"
    "${SOURCEPATH}/byte-code/chunk_file.h"
    "${SOURCEPATH}/frontend/compiler.h"
    "${SOURCEPATH}/frontend/lexer.h"
    "${SOURCEPATH}/middle-end/chunk_optimizer.h"
    "${SOURCEPATH}/language-models/object.h"
    "${SOURCEPATH}/language-models/value.h"
    "${SOURCEPATH}/language-models/data-structures/dynamic_value_array.h"
    "${SOURCEPATH}/language-models/data-structures/value_hash_table.h"
    )
endif()

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
    if(CLX_DEBUG_PRINT_BYTECODE)
        add_compile_definitions(DEBUG_PRINT_CODE)
    endif()
    if(CLX_DEBUG_TRACE_EXECUTION)
        add_compile_definitions(DEBUG_TRACE_EXECUTION)
    endif()
    if(CLX_DEBUG_STRESS_GARBAGE_COLLECTOR)
        add_compile_definitions(DEBUG_STRESS_GC)
    endif()
    if(CLX_DEBUG_LOG_GARBAGE_COLLECTOIN)
        add_compile_definitions(DEBUG_LOG_GC)
    endif()
    add_compile_definitions(BUILD_DEBUG)
endif()

# for including the cellox_config.h file
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_BINARY_DIR}/src)

if(NOT CMAKE_BUILD_TYPE MATCHES "Debug")    

    # Sets properties for the package created using cpack
    set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
    set(CPACK_PACKAGE_VERSION "${Cellox_VERSION_MAJOR}.${Cellox_VERSION_MINOR}.${Cellox_VERSION_PATCH}")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_NAME} Compiler")
    set(CPACK_BUILD_SOURCE_DIRS "${PROJECT_BINARY_DIR}/src")
    set(CPACK_PACKAGE_VENDOR ${PROJECT_VENDOR})
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}_${Cellox_VERSION_MAJOR}.${Cellox_VERSION_MINOR}")
    
    # Sets properties for NSIS (nullsoft scriptable install system - https://sourceforge.net/projects/nsis/)
    set(CPACK_NSIS_HELP_LINK ${PROJECT_HELP_PAGE})    
    set(CPACK_NSIS_MODIFY_PATH "ON")
    set(CPACK_NSIS_MUI_ICON "${PROJECT_SOURCE_DIR}/icons/file_type_cellox.ico")
    set(CPACK_NSIS_MUI_UNIICON "${PROJECT_SOURCE_DIR}/icons/file_type_cellox.ico")
    set(CPACK_NSIS_PACKAGE_NAME "${PROJECT_NAME}_${Cellox_VERSION_MAJOR}.${Cellox_VERSION_MINOR}_Installer")
    set(CPACK_NSIS_URL_INFO_ABOUT ${PROJECT_PAGE}) 
    
    # Includes the CPack module 
    include(CPack)

    # Install destinations
    install(TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()
