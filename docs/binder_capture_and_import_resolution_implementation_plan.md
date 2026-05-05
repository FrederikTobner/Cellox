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

### Design Stance: Single-File First, Project Mode Optional

The language should not force a project layout, naming convention, casing rule, or build system.
That means import resolution should be capability-based, not convention-based.

Recommended interpretation:

- Keep single-file execution as the primary model.
- Treat "project mode" as an optional runtime/tooling enhancement, not a language requirement.
- Resolve imports through explicit roots and deterministic precedence, never through implicit repo heuristics.

#### Why Single-File First is Strong

- Excellent for onboarding, scripting, and small utilities.
- Works naturally with "compile one file" workflows similar to C tools.
- Avoids premature lock-in to package metadata formats.
- Keeps the language timeless: source file + explicit imports stays valid across tooling eras.

#### Where Single-File Only Can Hurt Later

- Large multi-module codebases may want shared root configuration.
- Reproducible CI and packaging often benefit from explicit import root declarations.
- Cross-platform distribution can require predictable stdlib/application lookup without relying on CWD.

#### Low-Convention Compromise (Flags First)

Use a two-layer model:

1. Language/runtime layer (required):
    - Entry file directory root
    - Relative and absolute imports
    - Explicit stdlib root resolution chain

2. Tooling layer (optional):
    - Optional CLI flags to add extra import roots
    - Optional build integration for packaging

If no optional flags are provided, behavior remains fully defined by the language/runtime defaults.

This preserves freedom while still allowing scale when needed.

Practical direction:

- Prefer gcc/clang-style explicit flags over mandatory project metadata files.
- Keep import behavior reproducible in CI by passing the same flags in scripts.
- Allow build orchestration via user-defined scripts/programs (including future language-native build scripts), but keep that outside core language semantics.

### Compatibility Contract for Plan 3

To avoid future deprecations, freeze these guarantees early:

- Existing relative and absolute import semantics never change.
- Bare import fallback to entry-root remains valid unless user explicitly overrides roots.
- Resolution precedence is deterministic and documented.
- New mechanisms (CLI include path flags, package manager, optional tooling metadata) may add roots, but do not silently reorder existing precedence.

This contract enables incremental evolution without breaking old projects.

### Suggested Evolution Path

Phase A (now):

- Implement runtime root chain from this plan.
- Keep entry-root fallback as the default non-stdlib bare import behavior.

Phase B (optional later):

- Add explicit CLI include-root flags (repeatable).
- Keep them strictly opt-in.

Phase C (optional later):

- Add optional language-native build script conventions (or external script conventions) that translate to the same CLI flags.
- If tooling metadata files are ever supported, keep them strictly optional and equivalent to passing flags explicitly.

### Build Orchestration Position (Framework Side)

Preferred direction:

- Keep the Cellox compiler/runtime focused on compilation and execution primitives.
- Let a separate build framework/tool (written in Cellox or otherwise) orchestrate project builds.
- Do not require a mandatory project manifest for core compiler usage.

This is similar in spirit to C/C++ ecosystems:

- Compiler remains timeless and scriptable through flags.
- Build systems are replaceable layers on top (framework choice stays open).
- Users can still run single-file commands without adopting any framework.

#### Minimal Compiler Surface for Framework Authors

To support a robust external build framework, keep a stable, explicit CLI surface:

- Include/import roots: repeatable flags (for example, -I style)
- Stdlib root override flag
- Entry-root override flag
- Output path/format flags
- Build mode/optimization/debug flags

Frameworks should be thin translators from user intent to these flags.

#### Compatibility Rule

Any future framework feature should compile down to plain compiler invocations.
If a project can be built by the framework, it must also be representable as explicit compiler commands.
This prevents lock-in and keeps the core toolchain transparent.

### Toolchain Pluralism (Compiler + Stdlib Are Replaceable)

Design principle:

- The Cellox language is the specification.
- Compilers and standard libraries are implementations.
- No single implementation should be mandatory for ecosystem participation.

This enables a Unix-like model where defaults exist, but alternatives remain first-class.

#### Policy Goals

- Allow multiple compiler implementations to coexist.
- Allow multiple stdlib implementations (or profiles) to coexist.
- Keep default implementation convenient, but never exclusive.
- Avoid legal and technical lock-in patterns.

#### Runtime/CLI Implications

Support explicit selection/override points instead of hard-coding one global default:

- Stdlib root override (already aligned with Plan 3)
- Optional stdlib profile identifier (for example, core/posix/embedded) as a flag-level concern
- Compiler identification/version reporting command for tooling interoperability

No requirement that a compiler distribution must ship a specific stdlib implementation.

#### Compatibility Contract (Important)

To make replaceability practical, standardize interfaces rather than implementations:

- Language semantics and syntax are implementation-independent.
- Module/import resolution semantics are spec-defined; path roots are configurable.
- A minimum "core stdlib contract" (APIs + behavior) is documented and versioned.
- Conformance tests validate compiler and stdlib compatibility.

This is the mechanism that lets others build better implementations without fragmentation.

#### Recommended Governance Shape

- Maintain an open conformance suite.
- Version the language spec and core-stdlib contract separately.
- Permit extensions, but require feature flags/capabilities so portability remains visible.

This keeps innovation open while preserving ecosystem coherence.

---

## Delivery Order (Low Risk)

1. Plan 2 (pointer ownership fix)
2. Plan 1 (binder capture scope semantics)
3. Plan 3 (runtime root resolution + install alignment)

This keeps compiler/runtime changes isolated and makes regressions easier to bisect.

## Out of Scope (Future)

- Capture-by-reference binder semantics
- Package manager integration or package lock behavior
- Build-system-specific conventions
