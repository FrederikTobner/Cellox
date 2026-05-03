# Standard Library Plan

## Goal

Provide a `stdlib/` directory of Cellox source files that wrap and organise
the existing native functions into cohesive, importable modules. User code
`import`s from the stdlib just like any other module — the stdlib is plain
Cellox, not C.

---

## Current state

All built-in capability is exposed as **global native functions** that the VM
registers before any user code runs:

| Category | Native names |
|----------|-------------|
| I/O | `printf`, `read_line`, `read_key`, `read_file`, `write_to_file`, `append_to_file` |
| Math | `cosine`, `sine`, `tangent`, `exponential`, `logarithm`, `logarithm10`, `random` |
| Strings | `strlen`, `string_hash`, `string_replace_at`, `asci_to_num`, `num_to_asci` |
| Arrays | `array_length` |
| Reflection | `class_of`, `size_of` |
| Platform | `on_linux`, `on_macOS`, `on_windows` |
| System | `clock`, `exit`, `system`, `wait` |

Problems with the current approach:
- All names live in the global scope — naming collisions are always possible.
- No discoverability: a user cannot inspect what `cosine` belongs to.
- Related functions are scattered without structure.
- There is no idiomatic way to compose or extend bundled behaviour.

---

## Design principles

1. **Pure Cellox** — every stdlib file is `.clx`. No new C code is required for
   the stdlib itself. The native functions remain exactly as they are; the
   stdlib is a thin Cellox wrapper layer over them.
2. **Opt-in via `import`** — nothing from the stdlib is injected into user
   programs unless they explicitly import it. Global native functions stay
   available for backwards compatibility but the stdlib provides the preferred
   API going forward.
3. **Namespaced through classes** — each module exposes one (or a few)
   classes with `static`-style methods. This groups related functions, enables
   dot-notation (`Math.sin(x)`), and makes the namespace explicit.
4. **Installed alongside the binary** — the build system copies the stdlib
   files to a well-known location (e.g. `<install_prefix>/lib/cellox/stdlib/`)
   at install time. At runtime, `module_loader` resolves `"stdlib/math.clx"`
   relative to that prefix so users can write `import { Math } from "stdlib/math.clx"`.
5. **Tested** — each stdlib module gets at least one e2e test that imports it
   and exercises the wrapped functions.

---

## Proposed module layout

```
stdlib/
├── array.clx          → Array  class  (array_length, slicing helpers)
├── io.clx             → IO     class  (read_line, read_key, read_file, write_to_file, append_to_file)
├── math.clx           → Math   class  (sin, cos, tan, exp, log, log10, random)
├── os.clx             → OS     class  (on_linux, on_macOS, on_windows, system, exit, wait, clock)
└── string.clx         → Str    class  (len, hash, replace_at, char_code, from_char_code)
```

---

## Module design (per file)

### `stdlib/math.clx`

```cellox
export fun sin(x)    { return sine(x); }
export fun cos(x)    { return cosine(x); }
export fun tan(x)    { return tangent(x); }
export fun exp(x)    { return exponential(x); }
export fun log(x)    { return logarithm(x); }
export fun log10(x)  { return logarithm10(x); }
export fun rand()    { return random(); }
```

Usage:
```cellox
import { sin, cos } from "stdlib/math.clx";
printf("{}\n", sin(3.14159));
```

### `stdlib/io.clx`

```cellox
export fun print(fmt)              { printf(fmt); }
export fun println(fmt, value)     { printf(fmt, value); }
export fun input_line()            { return read_line(); }
export fun input_key()             { return read_key(); }
export fun file_read(path)         { return read_file(path); }
export fun file_write(path, text)  { return write_to_file(path, text); }
export fun file_append(path, text) { return append_to_file(path, text); }
```

### `stdlib/string.clx`

```cellox
export fun str_len(s)              { return strlen(s); }
export fun str_hash(s)             { return string_hash(s); }
export fun str_replace_at(s, i, c) { return string_replace_at(s, i, c); }
export fun char_code(c)            { return asci_to_num(c); }
export fun from_char_code(n)       { return num_to_asci(n); }
```

### `stdlib/array.clx`

```cellox
export fun arr_length(a) { return array_length(a); }
```

### `stdlib/os.clx`

```cellox
export fun is_linux()     { return on_linux(); }
export fun is_macOS()     { return on_macOS(); }
export fun is_windows()   { return on_windows(); }
export fun os_clock()     { return clock(); }
export fun os_wait(s)     { return wait(s); }
export fun os_exit(code)  { return exit(code); }
export fun os_system(cmd) { return system(cmd); }
```

---

## Stdlib resolution in `module_loader`

Today `module_loader` resolves import paths relative to the importing file's
directory. An `import "stdlib/math.clx"` from an arbitrary user file would
look for `math.clx` in `<user_dir>/stdlib/` — which is usually wrong.

### Change needed

Add a **stdlib search path** so that bare `stdlib/` prefixes resolve to the
installed location regardless of where user code lives. Two lookup steps:

1. Resolve relative to the importing file (existing behaviour — unchanged).
2. If the file is not found **and** the path starts with `stdlib/`, resolve
   relative to the stdlib install directory. The install directory is baked in
   at compile time as `CELLOX_STDLIB_DIR` (set by CMake, analogous to how
   `cellox_config.h` already carries version info).

No new syntax is required; this is a pure `module_loader.c` change (~20 lines).

---

## CMake changes

1. **New top-level `stdlib/` directory** with its own `CMakeLists.txt`.
2. Install rule: `install(FILES ... DESTINATION lib/cellox/stdlib)`.
3. `cellox_config.h.in` gains a new entry:
   ```c
   #define CELLOX_STDLIB_DIR "@CELLOX_STDLIB_INSTALL_DIR@"
   ```
4. `CLX_BUILD_TESTS=ON` configures `CELLOX_STDLIB_DIR` to point at
   `${PROJECT_SOURCE_DIR}/stdlib` so tests find the files without installing.

---

## Implementation steps (ordered)

| # | Step | Files touched |
|---|------|--------------|
| 1 | Write the five `.clx` stdlib files | `stdlib/*.clx` (new) |
| 2 | Add `stdlib/CMakeLists.txt` with install rules | `stdlib/CMakeLists.txt` (new) |
| 3 | Add `add_subdirectory(stdlib)` to root `CMakeLists.txt` | `CMakeLists.txt` |
| 4 | Add `CELLOX_STDLIB_DIR` to `cellox_config.h.in` | `src/cellox_config.h.in`, `src/CMakeLists.txt` |
| 5 | Add stdlib fallback lookup to `module_loader.c` | `src/module_loader.c` |
| 6 | Write e2e tests that `import` stdlib modules | `test/e2e/stdlib_*.cc` + program fixtures |
| 7 | Update `README.md` / `src/README.md` | docs |

Steps 1–4 are independent and can be done in one pass.  
Step 5 (module_loader change) depends on step 4 (config header must exist).  
Step 6 depends on steps 1–5 being complete.

---

## Backwards compatibility

- All current native function names remain globally registered — existing
  programs continue to work unchanged.
- The stdlib is additive; no language syntax changes are required.
- The only migration path encouraged (not enforced) is preferring
  `import { Math } from "stdlib/math.clx"` over bare `cosine(x)` in new code.

---

## Open questions / future work

- **Extended collections** — a `stdlib/collections.clx` with `Stack`, `Queue`,
  and `HashSet` implemented in pure Cellox on top of arrays and hash tables.
- **Error handling** — a `stdlib/error.clx` with a base `Error` class once
  exception / result-type semantics are designed.
- **`import "stdlib/math.clx" as math`** shorthand — a future language
  feature that would allow `math.sin(x)` without importing each name individually.
