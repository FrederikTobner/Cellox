# Binder Capture and Import Resolution Implementation Plan

## Scope

This document defines a mid-level implementation plan for three targeted improvements:

1. Binder capture semantics for `iferror` / `catch` handlers
2. Safe ownership for `module_loader_set_stdlib_path()`
3. Runtime import resolution for stdlib + entry-file-root (outside source tree)

The plan is intentionally language-agnostic in structure but maps directly onto the current Cellox architecture.

## Architectural Constraints

The implementation must follow existing subsystem boundaries:

- Frontend parsing/lowering in `src/frontend/src/parsing/expression_parser.c`
- Scope close semantics in `src/frontend/src/parsing/statement_parser.c`
- Module graph orchestration in `src/module-loading/src/module_loader.c`
- Path policy in `src/module-loading/src/module_path.c`
- Build/install contracts in `stdlib/CMakeLists.txt`, `src/base/cellox_config.h.in`, and `src/module-loading/CMakeLists.txt`

No build system abstraction layer should be introduced. The design must remain simple and composable.

---

## Plan 1: Binder Capture Semantics (Value-Only for Now)

### Goals

- Closures may capture binder variable declared in `|name|` handlers.
- Captures observe the final handler result value.
- Binder variable remains block-scoped.
- Binder local remains mutable inside the handler block.
- Current architecture is preserved (no parser rewrite).

### Current Hotspot

- `expression_parser_parse_pipe_bound_handler(...)` in `expression_parser.c`
- It currently writes result via `OP_SET_LOCAL` and manually unwinds scope metadata.

### Strategy

Keep the current lowering shape, but make scope-exit semantics explicit for captured binder locals.

### Pseudo Code

```text
function parse_pipe_bound_handler(precedence):
    consume('|')
    consume(identifier as boundName)
    consume('|')

    compiler_begin_scope()
    compiler_add_local(boundName)
    compiler_mark_initialized()
    boundSlot = CURRENT.localCount - 1

    parse handler expression or block

    # handler result is on top-of-stack
    emit(OP_SET_LOCAL, boundSlot)   # binder slot = final handler result
    emit(OP_POP)                    # drop temp top copy

    local = CURRENT.locals[boundSlot]
    if local.isCaptured:
        emit(OP_DUP)                # preserve expression result
        emit(OP_CLOSE_UPVALUE)      # close binder slot and pop one copy

    CURRENT.scopeDepth -= 1
    CURRENT.localCount -= 1
```

### Notes

- This keeps semantics simple: "capture by value" means captured closure observes the final bound value at scope exit.
- Object values remain pointer-like runtime values (same behavior as existing VM value model).

### Tests to Add

- Capture binder in nested closure and invoke later.
- Binder mutation inside handler reflects in captured closure result.
- Binder is not visible outside handler scope.
- Non-captured binder path still returns correct expression value.

---

## Plan 2: Safe Ownership for `module_loader_set_stdlib_path`

### Goals

- Remove dangling-pointer risk.
- Keep API surface minimal and backward compatible.

### Current Hotspot

- `module_loader_set_stdlib_path(char const * path)` stores raw pointer globally.

### Strategy

Internally own a duplicated copy and replace safely on each call.

### Pseudo Code

```text
global stdlib_path_override_owned = NULL

function module_loader_set_stdlib_path(path):
    if path == NULL:
        free(stdlib_path_override_owned)
        stdlib_path_override_owned = NULL
        return

    new_copy = strdup(path)
    if new_copy == NULL:
        # keep previous value unchanged
        return

    free(stdlib_path_override_owned)
    stdlib_path_override_owned = new_copy

function module_loader_get_stdlib_path():
    if stdlib_path_override_owned != NULL:
        return stdlib_path_override_owned
    return fallback_default
```

### Notes

- Keep single-threaded assumptions (same as current loader model).
- Add small test that passes temporary storage and validates path still works after original storage is gone.

---

## Plan 3: Runtime Import Resolution (stdlib + entry-file-root)

### Goals

- Run outside source tree by default.
- Support `import { x } from "stdlib/foo.clx"`.
- Support bare imports from entry file root for now.
- Leave extension point for future package roots.

### Required Defaults

- Entry-root = directory of the entry file path.
- Install layout target = `bin/Cellox` and `lib/stdlib/*.clx` (or equivalent platform path).

### Resolution Policy (Ordered)

1. Explicit runtime override via `module_loader_set_stdlib_path`
2. Environment override (e.g. `CELLOX_STDLIB_DIR`)
3. Executable-relative candidates
4. Compile-time fallback from generated config

For import forms:

- Absolute import path: use as-is
- Relative path (`./`, `../`): resolve against importer file directory
- Bare path:
  - if prefix `stdlib/`: resolve against stdlib root
  - else: resolve against entry-root

### Pseudo Code

```text
function resolve_effective_stdlib_root():
    if loader_override != NULL:
        return loader_override

    env = getenv("CELLOX_STDLIB_DIR")
    if env exists and non-empty:
        return env

    exe_dir = os_get_executable_dir()
    for candidate in [
        join(exe_dir, "../lib/stdlib"),
        join(exe_dir, "../lib/cellox/stdlib")
    ]:
        if path_exists(candidate):
            return candidate

    return CELLOX_STDLIB_DIR  # from generated config

function resolve_import(importer_path, entry_root, import_path, stdlib_root):
    if is_absolute(import_path):
        return import_path

    if starts_with_relative_prefix(import_path):
        return join(dirname(importer_path), import_path)

    if starts_with(import_path, "stdlib/"):
        rel = strip_prefix(import_path, "stdlib/")
        return join(stdlib_root, rel)

    return join(entry_root, import_path)
```

### Build/Install Alignment

- Keep stdlib installation in `stdlib/CMakeLists.txt`.
- Remove dependency on source-tree-only `CLX_STDLIB_PATH` in module-loading compile definitions.
- Ensure generated config fallback points to install-time stdlib location for packaged builds.

### Tests to Add

- Bare stdlib imports from temp entry file.
- Bare project-root import from entry directory while running from a different CWD.
- Env override precedence over executable-relative fallback.
- Runtime override precedence over all defaults.

---

## Delivery Order (Low Risk)

1. Plan 2 (pointer ownership fix)
2. Plan 1 (binder capture scope semantics)
3. Plan 3 (runtime root resolution + install alignment)

This keeps compiler/runtime changes isolated and makes regressions easier to bisect.

## Out of Scope (Future)

- Capture-by-reference binder semantics
- Configurable include root lists via command line
- Package manager integration or package lock behavior
- Build-system-specific conventions
