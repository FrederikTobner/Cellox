# clx_model — Value and object model

**CMake target:** `clx_model`  
**Sources:** `value.c`, `object.c`, `data-structures/dynamic_value_array.c`, `data-structures/value_hash_table.c`  
**Depends on:** `clx_base`

## Purpose

`clx_model` defines the runtime representation of all Cellox values and heap
objects, together with the data structures that operate on them.

| File | Description |
|------|-------------|
| `value.h/.c` | `value_t` tagged union (or NaN-boxed double), `native_function_t` typedef, value printing. |
| `object.h/.c` | Heap-allocated objects: strings, functions, closures, classes, instances, upvalues, arrays. Injection hooks for GC integration (`object_set_vm_string_table`, `object_set_vm_objects`, `object_set_gc_guard_hooks`). |
| `data-structures/dynamic_value_array.h/.c` | Growable array of `value_t` (used for constants in chunks). |
| `data-structures/value_hash_table.h/.c` | Open-addressing hash table mapping `object_string_t*` → `value_t` (used for globals, string interning, instance fields). |

## GC integration hooks

`object.c` deliberately does **not** `#include` the virtual-machine header.
Instead it receives three callbacks at startup via:

```c
object_set_vm_string_table(value_hash_table_t *); // string intern table
object_set_vm_objects(object_t **);               // GC root list head
object_set_gc_guard_hooks(push_fn, pop_fn);       // stack pinning during allocation
```

These are wired by `virtual_machine_init()` inside `clx_backend`.
