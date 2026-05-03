# clx_bytecode — Bytecode representation

**CMake target:** `clx_bytecode`  
**Sources:** `chunk.c`, `chunk_disassembler.c`, `chunk_file.c`  
**Depends on:** `clx_model`

## Purpose

`clx_bytecode` defines the chunk IR that the compiler writes and the VM
executes. It is also responsible for serialising chunks to `.cxcf` files and
reading them back.

| File | Description |
|------|-------------|
| `chunk.h/.c` | `chunk_t` — dynamic array of bytecode instructions plus a constant pool. `chunk_set_gc_guard_hooks` injection point prevents GC from collecting newly-added constants before the instruction is emitted. |
| `chunk_disassembler.h/.c` | Human-readable bytecode listing (used in `DEBUG_PRINT_CODE` / `DEBUG_TRACE_EXECUTION` builds and by the standalone disassembler tool). |
| `chunk_file.h/.c` | Binary serialisation / deserialisation of chunks to `.cxcf` (Cellox compiled-file) format. |

## Generated header

`cellox_config.h` is generated from `cellox_config.h.in` into the CMake
binary directory. `clx_bytecode` exposes that directory as a `PUBLIC`
include so all downstream targets can `#include "cellox_config.h"` without
any extra `target_include_directories` call.
