# src

This directory contains all source code for the Cellox interpreter. It is
organised into focused sub-libraries that have strict, one-way dependency
edges. Each sub-library is a CMake `STATIC` library target created through
the [`cellox_add_library`](../cmake/modules/CelloxHelpers.cmake) helper.

## Directory layout

```
src/
├── backend/           → clx_runtime  — virtual machine, GC, memory, native functions
├── byte-code/         → clx_bytecode — chunk IR, disassembler, serialisation
├── frontend/          → clx_lex, clx_frontend — lexer, parser, compiler
├── language-models/   → clx_model    — value & object representation, hash-table, array
├── middle-end/        → clx_middleend — bytecode optimisation passes
├── common.c/h         ┐
├── string_utils.c/h   ┘ → clx_base   — foundational utilities (shared by all)
├── command_line_argument_parser.c/h ┐
├── initializer.c/h                  ┤ → clx_driver — application-level glue
├── module_loader.c/h                ┘
└── main.c             → Cellox executable (links cellox_all)
```

## Dependency graph

```
clx_base
  ├── clx_model
  │     └── clx_bytecode
  │           ├── clx_middleend
  │           │     └── clx_frontend (also needs clx_lex)
  │           └── clx_runtime
  └── clx_lex
        └── clx_frontend
                └── clx_driver
                      └── Cellox (executable)
```

`cellox_all` is a convenience `INTERFACE` target that exposes `clx_driver` (and
therefore the entire graph) behind a single link target used by the benchmark
runner, disassembler, and test suites.

## Adding a new library

1. Create a sub-directory under `src/`.
2. Add a `CMakeLists.txt` using `cellox_add_library(...)`.
3. `add_subdirectory(...)` it in this file, after its dependencies.
