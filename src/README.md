# src

This directory contains all source code for the Cellox interpreter. It is
organised into focused sub-libraries that have strict, one-way dependency
edges. Each sub-library is a CMake `STATIC` library target created through
the [`cellox_add_library`](../cmake/modules/CelloxHelpers.cmake) helper.

## Directory layout

```
src/
├── backend/           → clx_backend        — virtual machine, GC, memory, native functions
├── base/              → clx_base           — foundational utilities (shared by all)
├── byte-code/         → clx_bytecode       — chunk IR, disassembler, serialisation
├── conditionals/                           — Conditional compilation layer currently for the compiler and host OS
│   ├── compiler/      → clx_toolchain      — compiler-specific attribute macros
│   └── os/            → clx_os             — OS abstraction (fs, path, stdio, temp, time)
├── driver/            → clx_driver         — application-level orchestration
├── frontend/          → clx_lex, clx_frontend — lexer, parser, compiler
├── language-models/   → clx_model          — value & object representation, hash-table, array
├── middle-end/        → clx_middleend      — bytecode optimisation passes
├── module-loading/    → clx_module_loading — module graph resolution & stdlib lookup
├── utils/             → clx_utils          — small shared helper routines
└── main.c             → Cellox executable  (links cellox_all)
```
## Adding a new library

1. Create a sub-directory under `src/`.
2. Add a `CMakeLists.txt` using `cellox_add_library(...)`.
3. `add_subdirectory(...)` it in this file, after its dependencies.
