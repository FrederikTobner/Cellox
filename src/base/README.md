# clx_base — Core shared definitions

**CMake target:** `clx_base`

`clx_base` provides the foundational shared definitions used across the rest of
the codebase. It is the lowest-level Cellox library and exposes the common
types, compiler exit codes, and portability/compiler-attribute integration that
other modules build on.

At a high level, this module is where project-wide basics live:

- `base/common.h` defines common scalar types, exit codes, and shared macros.
- `base/internal/macros.h` contains private helper macros that should not be
  consumed directly outside the base layer.
- `cellox_config.h.in` owns the generated build-time configuration header.
