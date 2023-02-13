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
    # Debug options
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
    add_compile_definitions(BUILD_TYPE_DEBUG)
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

    # CPACK - section

    # CPack metadata
    set(PROJECT_VERSIONED_NAME "${PROJECT_NAME}_${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH}") 
    set(PROJECT_PAGE "https://github.com/FrederikTobner/Cellox")
    set(PROJECT_HELP_PAGE "https://github.com/FrederikTobner/Cellox/wiki")
    set(PROJECT_VENDOR "Frederik Tobner")
    set(PROJECT_CONTACT "f.tobner@gmail.com")
    
    # Sets properties for the package created using cpack - https://cmake.org/cmake/help/book/mastering-cmake/chapter/Packaging%20With%20CPack.html
    
    # Setting source directory
    set(CPACK_BUILD_SOURCE_DIRS "${PROJECT_BINARY_DIR}/src")
    # Creates a desktop shortcut
    set(CPACK_CREATE_DESKTOP_LINKS ${PROJECT_NAME})
    # Sets the package description
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_NAME} Compiler")
    # Creates a short cut in the start menu
    set(CPACK_PACKAGE_EXECUTABLES ${PROJECT_NAME} ${PROJECT_VERSIONED_NAME})
    # Configurating the installation directory
    set(CPACK_PACKAGE_INSTALL_DIRECTORY ${PROJECT_VERSIONED_NAME})
    # Setting the name of the package
    set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
    # Setting the project vendor
    set(CPACK_PACKAGE_VENDOR ${PROJECT_VENDOR})
    # Setting the project version
    set(CPACK_PACKAGE_VERSION "${Cellox_VERSION_MAJOR}.${Cellox_VERSION_MINOR}.${Cellox_VERSION_PATCH}")
    # Add license as resouce
    set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")

    
    # Sets properties for NSIS (nullsoft scriptable install system - https://sourceforge.net/projects/nsis/)
    
    # Setting the branding text that is displayed at the bottom of the install window.   
    set(CPACK_NSIS_BRANDING_TEXT ${PROJECT_NAME})
    # Setting contact info
    set(CPACK_NSIS_CONTACT ${PROJECT_CONTACT})
    # Configurating help page
    set(CPACK_NSIS_HELP_LINK ${PROJECT_HELP_PAGE})
    # Add option to add the installed program to the path  
    set(CPACK_NSIS_MODIFY_PATH "ON")
    # Setting the icon if the installer
    set(CPACK_NSIS_MUI_ICON "${PROJECT_SOURCE_DIR}/icons/file_type_cellox.ico")
    # Setting the package name of the nsis package
    set(CPACK_NSIS_PACKAGE_NAME ${PROJECT_NAME})
    # About URL provided in the installer
    set(CPACK_NSIS_URL_INFO_ABOUT ${PROJECT_PAGE})    

    # Includes the CPack module 
    include(CPack)
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

# for including the cellox_config.h file
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_BINARY_DIR}/src)

if(NOT CMAKE_BUILD_TYPE MATCHES "Debug")
    # Configure install destination of the compiler
    install(TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()
