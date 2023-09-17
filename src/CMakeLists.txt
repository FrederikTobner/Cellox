# configure cellox config to show version
configure_file(cellox_config.h.in cellox_config.h)

cellox_add_compiler_compile_definitions()
cellox_add_os_compile_definitions()

if(UNIX)
    # Check dependecies under unix-like systems
    CHECK_INCLUDE_FILE("curses.h" CURSES_AVAILABLE)
    CHECK_INCLUDE_FILE("unistd.h" UNISTD_AVAILABLE)
    if(NOT ${CURSES_AVAILABLE})
        message(FATAL_ERROR "curses.h is required to build the compiler under unix-like systems. \
\   \   Please make sure it is available to the compiler and try again after that")
    endif() # curses.h not available
    if(NOT ${UNISTD_AVAILABLE})
        message(FATAL_ERROR "unistd.h is required to build the compiler under unix-like systems. \
\   \   Please make sure it is available to the compiler and try again after that")
    endif() # unistd.h not available
elseif(WIN32)
    # Check dependecies under windows
    CHECK_INCLUDE_FILE("conio.h" CONIO_AVAILABLE)
    CHECK_INCLUDE_FILE("windows.h" WINDOWS_AVAILABLE)
    if(NOT ${CONIO_AVAILABLE})
        message(FATAL_ERROR "conio.h is required to build the compiler under windows. \
\   \   Please make sure it is available to the compiler and try again after that")
    endif() # conio.h not available
    if(NOT ${WINDOWS_AVAILABLE})
        message(FATAL_ERROR "windows.h is required to build the compiler under windows. \
\   \   Please make sure it is available to the compiler and try again after that")
    endif() # windows.h not available
endif()

if(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]")
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
    set(CPACK_PACKAGE_VERSION "${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH}")
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

add_executable(${PROJECT_NAME} ${CELLOX_SOURCE_FILES} ${CELLOX_HEADER_FILES})

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

if(NOT CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]")
    # Configure install destination of the compiler
    install(TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()
