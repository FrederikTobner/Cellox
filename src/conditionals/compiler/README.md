# clx_toolchain — Compiler/toolchain abstraction layer

**CMake target:** `clx_toolchain`

`clx_toolchain` exposes compiler-specific attribute and portability macros to
the rest of the project through a single CMake target. The concrete
implementation is selected during configuration based on the active host C
compiler.

At a high level, this module hides compiler-specific details behind a stable
public include surface so other libraries can use things like inlining or
purity annotations without carrying compiler-specific conditionals.
