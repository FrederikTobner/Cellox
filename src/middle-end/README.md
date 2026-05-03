# clx_middleend — Bytecode optimisation

**CMake target:** `clx_middleend`  
**Sources:** `chunk_optimizer.c`  
**Depends on:** `clx_bytecode`

## Purpose

`clx_middleend` houses post-compilation bytecode optimisation passes that
operate on a `chunk_t` before it is handed to the VM (or written to a
`.cxcf` file).

| File | Description |
|------|-------------|
| `chunk_optimizer.h/.c` | Peephole / constant-folding passes over bytecode. Called from the compiler after a function chunk is finalised. |

New optimisation passes should be added here and exposed through
`chunk_optimizer.h`.
