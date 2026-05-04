# CelloxHelpers.cmake
# ─────────────────────────────────────────────────────────────────────────────
# Convenience wrappers for defining Cellox static libraries and executables.
# Every target created through these helpers gets the project-wide include root
# (src/) and inherits the compiler/OS compile-definitions that were set at the
# top level.
# ─────────────────────────────────────────────────────────────────────────────

# cellox_add_library(
#   <name>
#   SOURCES   file1.c [file2.c ...]
#   [DEPENDENCIES  target1 [target2 ...]]
#   [INCLUDE_DIRS  dir1   [dir2   ...]]   # extra PUBLIC include dirs
#   [COMPILE_DEFS  DEF1   [DEF2   ...]]   # extra PUBLIC compile definitions
# )
#
# Creates a STATIC library called <name>, wires SOURCES and links DEPENDENCIES
# with PUBLIC visibility so transitive consumers see all headers.
function(cellox_add_library NAME)
    cmake_parse_arguments(
        ARG                      # prefix
        ""                       # options (none)
        ""                       # single-value keywords (none)
        "SOURCES;DEPENDENCIES;INCLUDE_DIRS;COMPILE_DEFS"  # multi-value keywords
        ${ARGN}
    )

    if(NOT ARG_SOURCES)
        message(FATAL_ERROR "cellox_add_library(${NAME}): SOURCES must not be empty")
    endif()

    if(NOT DEFINED CLX_COMPILER_PUBLIC_INCLUDE_DIR)
        set(CLX_COMPILER_PUBLIC_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/src/conditionals/compiler/include")
    endif()
    if(NOT DEFINED CLX_COMPILER_IMPL_INCLUDE_DIR)
        set(CLX_COMPILER_IMPL_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/src/conditionals/compiler/impl/generic/include")
    endif()

    add_library(${NAME} STATIC ${ARG_SOURCES})

    # Every library can include from the project src/ root
    target_include_directories(${NAME} PUBLIC
        "${CMAKE_SOURCE_DIR}/src"
        "${CLX_COMPILER_PUBLIC_INCLUDE_DIR}"
        "${CLX_COMPILER_IMPL_INCLUDE_DIR}"
        ${ARG_INCLUDE_DIRS}
    )

    if(ARG_DEPENDENCIES)
        target_link_libraries(${NAME} PUBLIC ${ARG_DEPENDENCIES})
    endif()

    if(ARG_COMPILE_DEFS)
        target_compile_definitions(${NAME} PUBLIC ${ARG_COMPILE_DEFS})
    endif()
endfunction()


# cellox_add_executable(
#   <name>
#   SOURCES   file1.c [file2.c ...]
#   [DEPENDENCIES  target1 [target2 ...]]
#   [COMPILE_DEFS  DEF1   [DEF2   ...]]   # PRIVATE compile definitions
#   [PRECOMPILE_COMMON_HEADER]            # enable common.h PCH (only for the main executable)
# )
#
# Creates an executable, links its DEPENDENCIES with PRIVATE visibility.
function(cellox_add_executable NAME)
    cmake_parse_arguments(
        ARG
        "PRECOMPILE_COMMON_HEADER"   # option
        ""
        "SOURCES;DEPENDENCIES;COMPILE_DEFS"
        ${ARGN}
    )

    if(NOT ARG_SOURCES)
        message(FATAL_ERROR "cellox_add_executable(${NAME}): SOURCES must not be empty")
    endif()

    if(NOT DEFINED CLX_COMPILER_PUBLIC_INCLUDE_DIR)
        set(CLX_COMPILER_PUBLIC_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/src/conditionals/compiler/include")
    endif()
    if(NOT DEFINED CLX_COMPILER_IMPL_INCLUDE_DIR)
        set(CLX_COMPILER_IMPL_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/src/conditionals/compiler/impl/generic/include")
    endif()

    add_executable(${NAME} ${ARG_SOURCES})

    target_include_directories(${NAME} PRIVATE
        "${CMAKE_SOURCE_DIR}/src"
        "${CLX_COMPILER_PUBLIC_INCLUDE_DIR}"
        "${CLX_COMPILER_IMPL_INCLUDE_DIR}"
    )

    if(ARG_DEPENDENCIES)
        target_link_libraries(${NAME} PRIVATE ${ARG_DEPENDENCIES})
    endif()

    if(ARG_COMPILE_DEFS)
        target_compile_definitions(${NAME} PRIVATE ${ARG_COMPILE_DEFS})
    endif()

    # Precompiled-header (opt-in — only where common.h lives alongside the target)
    if(ARG_PRECOMPILE_COMMON_HEADER)
        if(MSVC)
            target_precompile_headers(${NAME} PUBLIC common.h common.c)
        else()
            target_precompile_headers(${NAME} PUBLIC common.h)
        endif()
    endif()
endfunction()
