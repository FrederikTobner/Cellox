# View + Generics Design Options for Cellox

## Context

Current state in Cellox:

- Views are implemented as array-based pipelines in [stdlib/view.clx](../stdlib/view.clx).
- Collections are class-based in [stdlib/collections.clx](../stdlib/collections.clx), with stack/queue/list/set-like abstractions.
- Module export parsing already supports `export class` in [src/module_loader.c](../src/module_loader.c#L234).
- Cellox is dynamically typed at runtime, so there is no existing static generic type system.

Goal:

- Support views over more than arrays, especially stack-backed views, without relying on duck typing hacks.
- Define a path that can evolve toward constraints/generics in a principled way.

Non-goals for now:

- Heavy dependency domains (audio, cryptography, GUI/windowing) are out of scope.

---

## Design 1: Nominal Interfaces + Constrained Generics (Java/C# Inspiration)

Inspired by:

- Java interfaces + bounded type parameters (`<T extends Interface>`)
- C# generic constraints (`where T : IInterface`)

### Core idea

Add a nominal interface system and constrained generics.

- `interface ViewSource<T> { size(); at(i); }`
- `class Stack<T> implements ViewSource<T>`
- `class ArrayView<TSource, TItem> where TSource : ViewSource<TItem>`

Views consume `ViewSource<T>` rather than concrete arrays.

### Example syntax sketch

```cellox
interface ViewSource<T> {
    size();
    at(i);
}

class Stack<T> : ViewSource<T> {
    // ...
    size() { ... }
    at(i) { ... }
}

class ArraySource<T> : ViewSource<T> {
    init(data) { this.data = data; }
    size() { return array_length(this.data); }
    at(i) { return this.data[i]; }
}

class View<T> {
    init(source) { this.source = source; }
}

export fun view_of<T, S: ViewSource<T>>(source) {
    return View(source);
}
```

### How stack view works

- `Stack<T>` implements `ViewSource<T>` directly.
- `view_of(myStack)` is valid by constraint.
- No duck typing and no special-case adapter functions required.

### Compiler/runtime impact

- Parser: interface declarations, generic parameter lists, `where` constraints.
- Semantic analysis: symbol resolution, interface conformance, constraint checks.
- Bytecode/VM: can use type erasure initially, no immediate monomorphization required.

### Pros

- Clear and familiar model.
- Strong API contracts.
- Good editor diagnostics and maintainability.

### Cons

- Medium-high implementation effort.
- Requires semantic analysis phase growth.

### Best fit

- Best long-term model if Cellox should feel like a modern OO language with clear contracts.

---

## Design 2: Traits / Typeclasses + Generic Algorithms (Rust/Haskell Inspiration)

Inspired by:

- Rust traits and trait bounds (`T: Trait`)
- Haskell typeclasses
- C++ execution policies and configurable algorithm behavior

### Core idea

Define behavior as traits and implement them for types.

- `trait Iterable<T>` or `trait ViewSource<T>`
- `impl ViewSource<T> for Stack<T>`
- Generic view combinators operate on traits, not classes.
- Traits are capability-oriented (Readable, Indexable, Sized, PushPop, etc.).
- Traits can define executable laws for testing and verification that can be applied on specific functions or the class itself. Laws can be both guarantees but also guards.
- Traits can support multi-method dispatch where behavior depends on multiple input types.
- Traits can be constrained using the Where syntax from Rust. For writting the where clauses we should support constraints like the predefined ones in C++.
- Algorithms can accept an execution policy/property object to control behavior, in order to avoid writting multiple functions for the same functionality, just with a different property, like foreach and foreach_par for example. Same thing just with a different property around the algorithm. The algorithms should have defaults, but still should be configurable as a whole.

### Example syntax sketch

```cellox
trait ViewSource<T> {
    size(source);
    at(source, i);

    law index_in_range(source, i) {
        if (i >= 0 and i < size(source)) {
            at(source, i) != null;
        }
    }
}

trait Intersect<A, B, Out> {
    intersect(a, b, policy);
}

impl<T> ViewSource<T> for Stack<T> {
    size(s) { return s.size(); }
    at(s, i) { return s.to_array()[i]; }
}

impl<T> ViewSource<T> for Array<T> {
    size(a) { return array_length(a); }
    at(a, i) { return a[i]; }
}

class ExecPolicy {
    init(mode, allocation, check_laws) {
        this.mode = mode;              // seq, parallel, vector
        this.allocation = allocation;  // default, arena, pooled
        this.check_laws = check_laws;  // true in tests, false in production
    }
}

fun map<T, U, S: ViewSource<T>>(source, fn, policy) {
    // generic algorithm honoring policy
}
```

### How stack view works

- Stack gets a ViewSource impl.
- Array gets a separate ViewSource impl.
- The same view and pipeline algorithms work for both by trait constraint.
- Laws can be executed in debug/test configurations.
- Policy can choose algorithm strategy and memory profile.

### Compiler/runtime impact

- Parser: traits/impl syntax + generics + laws + execution policy literals.
- Type system: trait resolution/coherence rules + multi-method candidate resolution.
- Codegen: either monomorphization or dictionary passing.
- Runtime: optional law execution hooks and policy-aware algorithm paths.

### Pros

- Very powerful abstraction model.
- Clean separation between data types and algorithms.
- Extensible: users can make custom collections work with views without modifying stdlib internals.
- Executable laws turn documentation into enforceable contracts.
- Multi-method traits model binary or N-ary behavior naturally.
- Policy object enables algorithm and memory behavior tuning without API explosion.

### Cons

- Highest complexity (trait solver design is non-trivial).
- Bigger learning curve for users.
- Requires careful guardrails so policy knobs do not become incoherent.

### Best fit

- Best if Cellox aims for highly composable, algorithm-centric standard library design and wants stronger contracts than conventional trait systems.

---

## Design 3: Erased Generics + Runtime-Enforced Constraints (TypeScript/Java Erasure Inspiration)

Inspired by:

- TypeScript generic syntax with erased runtime types.
- Java-style erased generics with runtime checks where needed.

### Core idea

Add generic syntax and constraint declarations, but erase them for runtime execution initially.

- Compile-time checks where possible.
- Runtime guard checks for boundary cases.
- Keep VM changes minimal in early phases.

### Example syntax sketch

```cellox
constraint ViewSource {
    size();
    to_array();
}

export fun view_of<TSource: ViewSource>(source) {
    // runtime guard inserted by compiler when needed
    return ArrayView(source.to_array());
}
```

### How stack view works

- `Stack` is declared to satisfy `ViewSource`.
- `view_of(stack())` compiles.
- No duck typing in user code, but runtime enforcement remains part of implementation in early phases.

### Compiler/runtime impact

- Parser: generic params + constraint declarations.
- Semantic: lightweight structural/nominal checking.
- VM: minimal changes, mostly metadata/guard support.

### Pros

- Fastest route to “generic-looking” APIs with constraints.
- Lower risk to VM internals.
- Good migration path toward stronger static checking later.

### Cons

- Less strict than fully static generics.
- Potential delayed errors when type information is incomplete.

### Best fit

- Best if you want iterative delivery and minimal disruption to current runtime architecture.

---

## Comparative Summary

| Criterion | Design 1: Interfaces + Constrained Generics | Design 2: Traits / Typeclasses | Design 3: Erased Generics + Runtime Constraints |
|---|---|---|---|
| Contract strength | High | Very high | Medium-high |
| Language complexity | Medium-high | High | Medium |
| VM impact | Medium | Medium-high | Low-medium |
| Performance ceiling | High | Very high | Medium-high |
| Time to first usable stack view | Medium | Medium-high | Fast |
| Long-term extensibility | High | Very high | High (if evolved) |
| Support for executable laws | Low | Very high | Medium |
| Multi-method dispatch ergonomics | Medium | Very high | Low-medium |
| Algorithm policy configurability | Medium | Very high | High |

---

## Recommended Path

If your priority is principled design without hacks and with a realistic implementation scope:

1. Start with Design 2 as the primary target, but stage it incrementally.
2. Deliver the trait core first:
    - capability-oriented trait taxonomy
    - dictionary-based dispatch
    - constrained generic algorithms for views
3. Add advanced layers next:
    - executable laws with test/debug execution modes
    - multi-method trait resolution
    - execution policy and algorithm property objects
4. Keep Design 3 implementation tactics where useful for early runtime pragmatism (erased internals first, stronger checks later).

This gives you explicit, composable contracts while staying aligned with Cellox's dynamic-friendly philosophy.

---

## Practical Next Step Proposal

Define a minimal capability trait set for views and algorithms:

- SizedSource
- RandomAccessSource
- PushPopSource
- SequenceSource

Implement these for Array wrapper, Stack, Queue, and LinkedList, then move view entry points to constrained generics. Add laws and policy parameters once base trait dispatch is stable.
