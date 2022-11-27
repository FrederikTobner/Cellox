# configure cellox config to show version
configure_file(cellox_config.h.in cellox_config.h)

add_executable(${PROJECT_NAME} ${CELLOXSOURCES} ${CELLOXHEADERS} main.c)

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

if(NAN_BOXING_ACTIVATED)
    add_compile_definitions(NAN_BOXING)
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
    
    # Sets properties for the package created using cpack and NSIS
    set(CPACK_NSIS_MUI_ICON "${PROJECT_DIRECTORY}/icons/file_type_cellox.ico")
    set(CPACK_NSIS_MODIFY_PATH "ON")
    set(CPACK_NSIS_URL_INFO_ABOUT ${PROJECT_PAGE})
    set(CPACK_NSIS_HELP_LINK ${PROJECT_HELP_PAGE})
    
    include(CPack)
    install(TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()
