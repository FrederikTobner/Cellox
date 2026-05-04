# CelloxHelpers.cmake
# ─────────────────────────────────────────────────────────────────────────────
# Convenience wrappers for defining Cellox static libraries and executables.
# Every target created through these helpers gets the project-wide include root
# (src/) and all per-module public include roots (src/<mod>/include/) so that
# cross-module public headers are always resolvable via "#include "<mod>/foo.h"".
#
# NOTE: clx_toolchain is NOT linked automatically. Targets that include
# compiler-attribute headers (base/common.h → clx_compiler/attributes.h) must
# declare it explicitly in their DEPENDENCIES list.
# ─────────────────────────────────────────────────────────────────────────────

# All per-module public include roots. Adding them here (rather than only via
# transitive link propagation) is necessary because the Cellox module graph has
# intentional cross-cutting header dependencies (e.g. language-models → backend
# allocator, byte-code → backend allocator) that do not follow the link graph.
set(CELLOX_MODULE_INCLUDE_ROOTS
    "${CMAKE_SOURCE_DIR}/src/base/include"
    "${CMAKE_SOURCE_DIR}/src/utils/include"
    "${CMAKE_SOURCE_DIR}/src/backend/include"
    "${CMAKE_SOURCE_DIR}/src/byte-code/include"
    "${CMAKE_SOURCE_DIR}/src/language-models/include"
    "${CMAKE_SOURCE_DIR}/src/middle-end/include"
    "${CMAKE_SOURCE_DIR}/src/module-loading/include"
    "${CMAKE_SOURCE_DIR}/src/frontend/include"
    "${CMAKE_SOURCE_DIR}/src/driver/include"
)

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

    add_library(${NAME} STATIC ${ARG_SOURCES})

    # Every library can include from the project src/ root and all module public headers
    target_include_directories(${NAME} PUBLIC "${CMAKE_SOURCE_DIR}/src" ${CELLOX_MODULE_INCLUDE_ROOTS} ${ARG_INCLUDE_DIRS})

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
# )
#
# Creates an executable, links its DEPENDENCIES with PRIVATE visibility.
function(cellox_add_executable NAME)
    cmake_parse_arguments(
        ARG
        ""   # options (none)
        ""
        "SOURCES;DEPENDENCIES;COMPILE_DEFS"
        ${ARGN}
    )

    if(NOT ARG_SOURCES)
        message(FATAL_ERROR "cellox_add_executable(${NAME}): SOURCES must not be empty")
    endif()

    add_executable(${NAME} ${ARG_SOURCES})

    target_include_directories(${NAME} PRIVATE "${CMAKE_SOURCE_DIR}/src" ${CELLOX_MODULE_INCLUDE_ROOTS})

    if(ARG_DEPENDENCIES)
        target_link_libraries(${NAME} PRIVATE ${ARG_DEPENDENCIES})
    endif()

    if(ARG_COMPILE_DEFS)
        target_compile_definitions(${NAME} PRIVATE ${ARG_COMPILE_DEFS})
    endif()
endfunction()
