# clx_backend — Virtual machine, GC, memory, native functions

**CMake target:** `clx_backend`  
**Sources:** `garbage_collector.c`, `memory_mutator.c`, `native_functions.c`, `virtual_machine.c`  
**Depends on:** `clx_bytecode` (+ `m` on UNIX)

## Purpose

`clx_backend` is the execution engine of Cellox. It intentionally does **not**
depend on `clx_frontend`; the compiler is injected via a function pointer at
startup.

| File | Description |
|------|-------------|
| `virtual_machine.h/.c` | Stack-based bytecode interpreter (`virtual_machine_interpret`). Manages the value stack, call stack, open upvalues, global table, and string intern table. |
| `garbage_collector.h/.c` | Tri-colour mark-and-sweep GC. The mark-roots phase is pluggable via `garbage_collector_set_mark_roots_hook` so the compiler's root set can be included without a direct dependency. |
| `memory_mutator.h/.c` | Central `memory_mutator_reallocate` wrapper; triggers GC when the heap threshold is exceeded. |
| `native_functions.h/.c` | Built-in native functions exposed to Cellox programs (clock, type queries, etc.). |

## Injection hooks

`clx_backend` breaks its only upward cycle (→ compiler) through three hooks
registered before `virtual_machine_init()` is called:

```c
virtual_machine_set_compile_fn(compiler_compile);         // compile source → chunk
garbage_collector_set_mark_roots_hook(compiler_mark_roots); // GC roots from compiler
```

`virtual_machine_init()` itself wires the object-model hooks:

```c
object_set_vm_string_table(&virtualMachine.strings);
object_set_vm_objects(&virtualMachine.objects);
object_set_gc_guard_hooks(virtual_machine_push, virtual_machine_pop);
chunk_set_gc_guard_hooks(virtual_machine_push, virtual_machine_pop);
```
