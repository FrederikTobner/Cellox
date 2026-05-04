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

# Bake the stdlib directory into the module-loading library so that bare imports
# like `import { abs } from "stdlib/math.clx"` resolve at runtime without the
# user needing to know the installation path.  The value can be overridden at
# runtime via module_loader_set_stdlib_path().
target_compile_definitions(clx_module_loading PRIVATE
    CLX_STDLIB_PATH="${PROJECT_SOURCE_DIR}/stdlib"
)
