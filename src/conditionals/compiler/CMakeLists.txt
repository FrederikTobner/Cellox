# compiler/CMakeLists.txt
# ─────────────────────────────────────────────────────────────────────────────
# Defines the clx_toolchain INTERFACE library.
#
# clx_toolchain exposes the host-toolchain attribute headers (CLX_ALWAYS_INLINE,
# CLX_HOT, etc.) chosen at configure time for the active C compiler.
#
# Link explicitly where compiler attributes are used:
#
#   target_link_libraries(my_target PUBLIC clx_toolchain)
#
# The two required variables are set before this subdirectory is added by
# cmake/conditionals/compiler/select.cmake:
#
#   CLX_COMPILER_PUBLIC_INCLUDE_DIR  – path to src/conditionals/compiler/include
#   CLX_COMPILER_IMPL_INCLUDE_DIR    – toolchain-specific impl header path
# ─────────────────────────────────────────────────────────────────────────────

if(NOT DEFINED CLX_COMPILER_PUBLIC_INCLUDE_DIR)
    message(FATAL_ERROR "CLX_COMPILER_PUBLIC_INCLUDE_DIR must be set before adding conditionals/compiler")
endif()
if(NOT DEFINED CLX_COMPILER_IMPL_INCLUDE_DIR)
    message(FATAL_ERROR "CLX_COMPILER_IMPL_INCLUDE_DIR must be set before adding conditionals/compiler")
endif()

add_library(clx_toolchain INTERFACE)

target_include_directories(clx_toolchain INTERFACE
    "${CLX_COMPILER_PUBLIC_INCLUDE_DIR}"
    "${CLX_COMPILER_IMPL_INCLUDE_DIR}"
)
