# clx_driver — Top-level interpreter orchestration

**CMake target:** `clx_driver`

`clx_driver` is the application-facing orchestration layer of Cellox. It sits
above the frontend, runtime, and module-loading code and turns high-level user
actions into concrete behaviour.

At a high level, this module is responsible for:

- parsing command-line arguments,
- selecting the requested execution mode,
- wiring together the frontend, backend, and module loader for normal program execution.

The main executable adds only the actual `main()` entry point on top of this
library.

## Sources

| File | Description |
|------|-------------|
| `command_line_argument_parser.h/.c` | Parses `argc`/`argv` into a `command_line_config_t`. Validates constraints and exits on bad input. |
| `initializer.h/.c` | Implements the three execution modes (REPL, run-from-file, compile-to-bytecode) and prints help/version output. |

## Command-line flags

| Flag | Field | Description |
|------|-------|-------------|
| `-h`, `--help` | `help` | Print usage and exit. |
| `-v`, `--version` | `version` | Print version and exit. |
| `-c`, `--compile` | `compile` | Compile to `.cxcf` bytecode instead of executing. |
| `-O0` … `-O3` | `optimizationLevel` | Bytecode optimisation level (default: none). |
| `--stdlib-dir <path>` | `stdlibDir` | Override the stdlib directory used for bare module imports. Equivalent to setting `CELLOX_STDLIB_DIR` for this invocation only. |

## Execution modes

1. **REPL** — no path argument; reads and evaluates lines interactively.
2. **Run from file** — executes the given `.clx` source or `.cxcf` bytecode file.
3. **Compile** — `-c` flag; compiles the source and writes a `.cxcf` file.
