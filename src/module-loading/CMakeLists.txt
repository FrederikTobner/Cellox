cellox_add_library(clx_module_loading
    SOURCES
        src/module_loader.c
        src/module_parser.c
        src/module_path.c
    DEPENDENCIES
        clx_base
        clx_utils
        clx_os
    INCLUDE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Bake a last-resort stdlib fallback path into the module-loading library.
# This is only reached when no runtime override, no CELLOX_STDLIB_DIR env var,
# and no exe-relative lib/cellox/stdlib directory is found.
# For dev/test builds this points to the source tree stdlib so no install step
# is needed.  Installed binaries are expected to find the stdlib via the
# exe-relative path instead (e.g. <prefix>/bin/../lib/cellox/stdlib).
target_compile_definitions(clx_module_loading PRIVATE
    CLX_STDLIB_PATH="${CELLOX_STDLIB_SOURCE_DIR}"
)
