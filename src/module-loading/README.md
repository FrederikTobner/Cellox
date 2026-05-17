# clx_module_loading — Module graph resolution

**CMake target:** `clx_module_loading`

`clx_module_loading` is responsible for Cellox source-level module handling. It
loads source files, resolves import paths, validates import/export usage, and
assembles the final source text that is passed into the compiler frontend.

At a high level, this module turns a file-based module graph into a compiler
input string while keeping filesystem handling and import parsing separate from
the frontend itself.

## Directory layout

```
module-loading/
├── include/module-loading/
│   └── module_loader.h     — public API (load program, set stdlib path)
├── internal/
│   ├── module_parser.h     — import/export statement parser
│   └── module_path.h       — path resolution helpers
└── src/
    ├── module_loader.c     — top-level orchestration + stdlib path resolution
    ├── module_parser.c     — parses import/export lines from source text
    └── module_path.c       — canonicalisation + import path resolution
```

## Internal components

| File | Responsibility |
|------|----------------|
| `module_loader.c` | Entry point. Maintains the set of already-loaded modules (load-once semantics), detects cycles, and concatenates assembled source texts. |
| `module_parser.c` | Parses `import` / `export` declarations from a raw source text and returns structured `import_spec_t` / `export_spec_t` lists. |
| `module_path.c` | Resolves a raw import path (relative or bare) to an absolute filesystem path and canonicalises it. |

## Standard library resolution order

Bare imports (paths that do not start with `.`, `/`, or a Windows drive letter)
are resolved against the standard library directory. The directory is chosen
using the following priority chain, stopping at the first match that points to
an existing path:

1. Explicit override via `module_loader_set_stdlib_path()` (wired from `--stdlib-dir`).
2. `CELLOX_STDLIB_DIR` environment variable.
3. `stdlib/` directory adjacent to the running interpreter executable.
4. Path compiled in at build time (`CLX_STDLIB_PATH` cmake variable).

## Public API

```c
// Load and assemble a module graph rooted at entryPath.
// Returns heap-allocated source text ready for the compiler, or NULL on error.
char * module_loader_load_program(char const * entryPath);

// Override the stdlib directory used for bare imports.
// Pass NULL to reset to the compiled-in default.
void module_loader_set_stdlib_path(char const * path);
```
