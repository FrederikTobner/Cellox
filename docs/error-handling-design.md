# Cellox Error Handling Design

## Goal

Design error handling for Cellox inspired by Zig:

- no exceptions as the primary mechanism
- errors are explicit values in the type/runtime model
- callers are forced to handle fallible operations
- syntax should be lightweight and readable
- runtime failures that are truly unrecoverable remain separate from ordinary domain errors

This document is a language design, not an implementation plan.

---

## Design Principles

1. Errors are values, not control-flow magic.
2. Fallibility must be visible at the call site.
3. The language should make the common case concise.
4. Recoverable errors and fatal VM/runtime faults must stay distinct.
5. The design must fit Cellox's current dynamic runtime and only later grow into richer static checking.

---

## Why Not Exceptions

Exceptions are a poor fit for the direction described so far because they:

- hide control flow
- make it easy to ignore failure paths
- complicate VM/runtime semantics substantially
- create pressure for stack unwinding, catch scopes, and destructor/finalizer semantics

Cellox already has explicit runtime failure reporting in the VM. That is appropriate for:

- invalid bytecode/runtime invariants
- illegal operations the program did not model as recoverable
- internal errors

But application/domain errors should become explicit and handled.

---

## High-Level Model

Cellox should have **error unions** and **error sets**.

Core idea:

- A function can return either a success value or an error value.
- The caller must unwrap, propagate, or transform the result.
- Errors belong to named error sets.

Conceptually:

```cellox
FileError!string
```

means:

- either a `string`
- or an error from `FileError`

This is directly inspired by Zig, but adapted to Cellox's more dynamic philosophy.

---

## Core Language Concepts

## 1. Error Sets

Error sets define named recoverable failure conditions.

Example:

```cellox
error FileError {
    NotFound,
    PermissionDenied,
    InvalidPath,
    ReadFailed,
}
```

Properties:

- Error names are symbolic, not numeric magic constants.
- Error sets are namespace-like groups.
- Error values are comparable and printable.

Example usage:

```cellox
printf("{}\n", FileError.NotFound);
```

### Design choice

Use named sets rather than ad-hoc strings because:

- diagnostics are better
- APIs become self-documenting
- later static checks become possible

---

## 2. Error Unions

A fallible result combines a normal value type with an error set.

Examples:

```cellox
fun read_text(path) -> FileError!string { ... }
fun parse_int(text) -> ParseError!number { ... }
fun connect(addr) -> NetworkError!Socket { ... }
```

If Cellox keeps lightweight syntax, the arrow could be optional in early versions, but the design target should be explicit return annotations for fallible functions.

### Dynamic-runtime interpretation

Even before full static typing exists, this syntax still matters:

- compiler marks the function as fallible
- VM/runtime produces a tagged result value
- callers must use fallible-call syntax or explicit handling

---

## 3. Error Return Values

A function returns an error explicitly:

```cellox
return error FileError.NotFound;
```

or, with shorter syntax:

```cellox
return FileError.NotFound;
```

Recommended rule:

- inside a function declared to return `ErrorSet!T`, returning a member of that error set is treated as an error return
- outside such a context, `FileError.NotFound` is just a plain error value

This keeps the syntax concise while still readable.

---

## 4. Forced Handling at Call Sites

A fallible call must not be silently used as a plain value.

That means this is illegal:

```cellox
var text = read_text("a.txt"); // invalid if read_text is fallible
```

The caller must choose one of the following:

1. Propagate the error.
2. Handle the error.
3. Provide a fallback/default.
4. Assert success and crash intentionally.

This is the core Zig-like property worth preserving.

---

## Proposed Syntax

## 1. `try` for propagation

```cellox
fun load_config(path) -> FileError!Config {
    var text = try read_text(path);
    return try parse_config(text);
}
```

Semantics:

- if the expression is success, unwrap the value
- if it is an error, return that error from the current function immediately

Requirements:

- current function must itself be fallible and compatible with the propagated error

---

## 2. `catch` for local handling

```cellox
var text = read_text(path) catch fun(err) {
    printf("config read failed: {}\n", err);
    return "";
};
```

or lighter syntax:

```cellox
var text = read_text(path) catch "";
```

Semantics:

- on success, unwrap value
- on error, evaluate handler/fallback

Recommended forms:

```cellox
expr catch fallback
expr catch fun(err) { ... }
```

This keeps the common case concise while still permitting richer handling.

---

## 3. `iferror` / pattern-style branching

Cellox should also have an explicit branching form when the code needs both success and error branches.

Example:

```cellox
iferror read_text(path) |err| {
    printf("error: {}\n", err);
} else |text| {
    printf("{}\n", text);
}
```

This is better than forcing everything through `catch` when both branches are substantial.

Alternative spelling options:

- `iferror expr |err| { ... } else |value| { ... }`
- `match`-style handling if pattern matching is added later

Recommended short-term design:

- add `iferror` as the dedicated branching construct

---

## 4. `orelse`-style shorthand

For optional/default-like usage, a shorter fallback operator may be desirable later:

```cellox
var port = parse_int(env("PORT")) orelse 8080;
```

This is optional. It overlaps with `catch fallback`, so it should only be added if it significantly improves readability.

Recommendation:

- do **not** add this in v1 of error handling
- keep `catch value` as the single fallback form

---

## 5. `must` / `expect` for intentional crash-on-error

Sometimes a program wants to assert that failure is impossible or fatal.

Example:

```cellox
var config = must read_text("config.clx");
```

Semantics:

- unwrap success
- if error, print the error and terminate like a runtime failure

This is useful for:

- tests
- scripts
- application startup

This keeps intentional fatal handling explicit, instead of silently defaulting to exceptions.

---

## Error Values at Runtime

Because Cellox is currently dynamically typed, the runtime likely needs a new value/object representation.

Suggested runtime shape:

- `error set` definitions become runtime objects or compiler-known symbols
- `error value` is a tagged object carrying:
  - error set id/name
  - variant name
  - optional payload in future versions
- `error union result` is represented as either:
  - plain success value plus compiler-side fallibility tracking
  - or a dedicated tagged runtime wrapper

Recommended v1 runtime representation:

- use a dedicated runtime wrapper object for fallible results

Conceptually:

```cellox
Result(success: true, value: ...)
Result(success: false, error: FileError.NotFound)
```

Even if the surface syntax is Zig-like, the VM can implement it using tagged objects initially.

Why this is pragmatic:

- minimal disruption to existing value model
- clearer interpretation rules in a dynamic runtime
- easier debugging

Later, the compiler can optimize away wrappers in common cases.

---

## Error Set Composition

Functions often need to propagate multiple error families.

Possible syntax:

```cellox
fun load(path) -> (FileError | ParseError)!Config { ... }
```

This should be supported eventually.

Alternative design:

```cellox
error ConfigLoadError {
    NotFound,
    PermissionDenied,
    InvalidSyntax,
}
```

Recommendation:

- v1 should support unioned error sets syntactically
- compiler can lower them to a normalized combined error representation

This is important for `try` to remain ergonomic across multi-stage pipelines.

---

## Error Payloads

Eventually it would be useful to allow payloads:

```cellox
error ParseError {
    InvalidNumber(text),
    UnexpectedToken(token, line),
}
```

But this adds a lot of complexity.

Recommendation:

- v1: no payloads
- v2: add optional payload-bearing variants once base error unions are stable

This mirrors the right complexity curve.

---

## Relationship to `null`

`null` and `error` must stay distinct.

- `null` means absence / empty / sentinel
- `error` means a recoverable failure

This is important because many dynamic languages blur these concepts and end up with weak APIs.

Examples:

- `Map.get(key)` may reasonably return `null`
- `file_read(path)` should return `FileError!string`

This distinction improves API clarity significantly.

---

## Relationship to VM Runtime Errors

Cellox should keep two layers of failure:

## 1. Recoverable language-level errors

Examples:

- file not found
- parse failed
- invalid user input
- network timeout

These use error unions and must be handled explicitly.

## 2. Fatal runtime errors

Examples:

- invalid bytecode invariant
- stack overflow
- illegal operation not modeled as recoverable
- VM internal bug

These remain VM/runtime errors and abort execution.

This distinction is critical. Do not try to model all current runtime faults as recoverable error unions immediately.

---

## Native Function Integration

Native functions are one of the most important integration points.

Today many native functions either:

- return plain values
- return `null`
- or trigger runtime failure paths

With the new design, fallible native functions should return explicit error unions.

Examples:

```cellox
fun file_read(path) -> FileError!string
fun file_write(path, text) -> FileError!bool
fun parse_number(text) -> ParseError!number
```

At the C layer, native functions should be able to construct:

- success results
- error results

This will likely require helper APIs in the VM/native boundary.

---

## Standard Library Style

The stdlib should adopt these rules:

1. I/O APIs are fallible by default.
2. Parsing APIs are fallible by default.
3. Collection operations should only be fallible where failure is meaningful.
4. Programmer errors should not be hidden as ordinary recoverable errors unless intentionally modeled.

Examples:

```cellox
fun read_text(path) -> FileError!string
fun write_text(path, text) -> FileError!bool
fun parse_int(text) -> ParseError!number
fun stack_pop(s) -> EmptyCollectionError!T
```

Whether `stack_pop` should be fallible or return `null` is a library design choice, but Cellox should now be able to model it explicitly.

---

## Syntax Summary

Recommended surface syntax for v1:

```cellox
error FileError {
    NotFound,
    PermissionDenied,
}

fun read_text(path) -> FileError!string {
    // ...
}

fun load(path) -> FileError!Config {
    var text = try read_text(path);
    return parse_config(text) catch fun(err) {
        printf("parse failed: {}\n", err);
        return error FileError.NotFound;
    };
}

var cfg = must load("config.clx");

iferror read_text("foo.txt") |err| {
    printf("{}\n", err);
} else |text| {
    printf("{}\n", text);
}
```

---

## Recommended V1 Feature Set

Implement first:

1. named error sets
2. error unions (`ErrorSet!T`)
3. `try` propagation
4. `catch` fallback/handler
5. `must` for explicit fatal unwrap
6. `iferror` branching
7. native function support for success/error results

Do not implement in v1:

1. payload-bearing errors
2. exception-style `throw/catch`
3. automatic implicit conversions between `null` and error
4. full pattern matching over error unions
5. stack unwinding semantics beyond ordinary return propagation

---

## Comparison to Zig

Things to keep from Zig:

- explicit fallibility
- forced handling
- `try`-style propagation
- compact readable syntax
- separation between recoverable errors and fatal failures

Things to adapt for Cellox:

- use runtime-wrapped result objects initially because Cellox is dynamic
- allow slightly more ergonomic branching forms like `iferror`
- keep syntax readable even without a full static type system

So the goal should be:

- **Zig philosophy, Cellox implementation style**

not a literal Zig clone.

---

## Open Questions

1. Should `must` print only the error value or also a source traceback?
2. Should fallible top-level code be allowed, or must top-level always terminate fatally on unhandled error?
3. Should error sets be nominal only, or can union composition create anonymous sets?
4. Should stdlib collection APIs prefer `null` or explicit error sets for empty-state operations?
5. Should `catch` be expression-only in v1, or also support statement blocks with richer local bindings?

---

## Recommendation

Adopt a Zig-inspired model with:

- `error` sets
- `ErrorSet!T` error unions
- `try`, `catch`, `must`, and `iferror`
- no exceptions
- no implicit swallowing of fallible results

This gives Cellox:

- explicit and disciplined error handling
- readable syntax
- strong stdlib API design
- a path to stronger compile-time enforcement later

without paying the complexity cost of exception machinery.
