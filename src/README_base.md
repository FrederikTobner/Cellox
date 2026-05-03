# clx_base — Foundational utilities

**CMake target:** `clx_base`  
**Sources:** `common.c`, `string_utils.c`  
**Depends on:** _(nothing)_

## Purpose

`clx_base` is the root of the dependency graph. It provides utilities that
every other library in Cellox needs:

| File | Description |
|------|-------------|
| `common.h/.c` | Shared macros, `bool` shim, platform guards, `FREE_ARRAY` / `GROW_CAPACITY` etc. |
| `string_utils.h/.c` | FNV-1a string hashing used by the value hash-table and object interning. |

Because this library sits at the bottom of the dependency tree it must **not**
`#include` any other project header. All other libraries link against it with
`PUBLIC` visibility so their own consumers transitively see `common.h`.
