# configure cellox config to show version
configure_file(cellox_config.h.in cellox_config.h)

if(CLX_OPTIMIZATION_PRINT_STATS)
    add_compile_definitions(CLX_OPTIMIZATION_PRINT_STATS)
endif()

if(CLX_OPTIMIZATION_VERBOSE_STATS)
    add_compile_definitions(CLX_OPTIMIZATION_VERBOSE_STATS)
endif()
if(NOT CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]")
    # ── CPack packaging (release builds only) ────────────────────────────────
    set(PROJECT_VERSIONED_NAME
        "${PROJECT_NAME}_${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH}")
    set(PROJECT_PAGE        "https://github.com/FrederikTobner/Cellox")
    set(PROJECT_HELP_PAGE   "https://github.com/FrederikTobner/Cellox/wiki")
    set(PROJECT_VENDOR      "Frederik Tobner")
    set(PROJECT_CONTACT     "f.tobner@gmail.com")

    set(CPACK_BUILD_SOURCE_DIRS           "${PROJECT_BINARY_DIR}/src")
    set(CPACK_CREATE_DESKTOP_LINKS        ${PROJECT_NAME})
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_NAME} Compiler")
    set(CPACK_PACKAGE_EXECUTABLES         ${PROJECT_NAME} ${PROJECT_VERSIONED_NAME})
    set(CPACK_PACKAGE_INSTALL_DIRECTORY   ${PROJECT_VERSIONED_NAME})
    set(CPACK_PACKAGE_NAME                ${PROJECT_NAME})
    set(CPACK_PACKAGE_VENDOR              ${PROJECT_VENDOR})
    set(CPACK_PACKAGE_VERSION
        "${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH}")
    set(CPACK_RESOURCE_FILE_LICENSE       "${PROJECT_SOURCE_DIR}/LICENSE")

    set(CPACK_NSIS_BRANDING_TEXT  ${PROJECT_NAME})
    set(CPACK_NSIS_CONTACT        ${PROJECT_CONTACT})
    set(CPACK_NSIS_HELP_LINK      ${PROJECT_HELP_PAGE})
    set(CPACK_NSIS_MODIFY_PATH    "ON")
    set(CPACK_NSIS_MUI_ICON       "${PROJECT_SOURCE_DIR}/icons/file_type_cellox.ico")
    set(CPACK_NSIS_PACKAGE_NAME   ${PROJECT_NAME})
    set(CPACK_NSIS_URL_INFO_ABOUT ${PROJECT_PAGE})

    include(CPack)
endif()

# ── foundational utilities (clx_base) ────────────────────────────────────────
cellox_add_library(clx_base
    SOURCES
        common.c
        string_utils.c
)

# ── Sub-libraries (each managed in their own CMakeLists.txt) ─────────────────
add_subdirectory(language-models)   # → clx_model
add_subdirectory(byte-code)         # → clx_bytecode
add_subdirectory(conditionals/os)   # → clx_os
add_subdirectory(middle-end)        # → clx_middleend
add_subdirectory(frontend)          # → clx_lex, clx_frontend
add_subdirectory(backend)           # → clx_runtime
add_subdirectory(module-loading)    # → clx_module_loading

cellox_add_library(clx_app
    SOURCES
        command_line_argument_parser.c
        initializer.c
    DEPENDENCIES
        clx_runtime
        clx_frontend
        clx_module_loading
)

# ── Interface alias — downstream tools link this one target ──────────────────
add_library(cellox_all INTERFACE)
target_link_libraries(cellox_all INTERFACE clx_app)

# ── Main executable ───────────────────────────────────────────────────────────
cellox_add_executable(${PROJECT_NAME}
    SOURCES
        main.c
    DEPENDENCIES
        cellox_all
    PRECOMPILE_COMMON_HEADER
)

if(NOT CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]")
    install(TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()

