# Error Handling Implementation Plan for Cellox

## Objective

Implement Zig-inspired explicit error handling in Cellox with:

- named error sets
- error unions (`ErrorSet!T`)
- forced handling of fallible results
- `try`, `catch`, `must`, and `iferror`
- no exceptions
- clear separation between recoverable language errors and fatal VM/runtime errors

This document translates the design in [docs/error-handling-design.md](./error-handling-design.md) into an implementation plan aligned with the current Cellox architecture.

---

## Current Architecture Constraints

1. The compiler is single-pass and emits bytecode directly while parsing.
2. There is no existing type-checking or semantic analysis pass.
3. Values currently represent numbers, booleans, null, and objects in [src/language-models/value.h](../src/language-models/value.h).
4. Native functions currently return a plain `value_t` and signal many failures either via `null` or VM runtime errors.
5. Fatal runtime failures are reported through `virtual_machine_runtime_error(...)` and terminate interpretation with `INTERPRET_RUNTIME_ERROR` in [src/backend/virtual_machine.h](../src/backend/virtual_machine.h).

Implication:

- Full “forced handling” cannot be implemented robustly with parser-only checks.
- A semantic/declaration pass will be needed once fallibility becomes part of function signatures.

---

## Recommended Rollout Strategy

Implement the feature in 6 stages:

1. Runtime representation for errors and fallible results.
2. Surface syntax for error sets and fallible operations.
3. Compiler lowering for `try`, `catch`, `must`, and `iferror`.
4. Semantic fallibility tracking and forced-handling checks.
5. Native-function and stdlib migration.
6. Diagnostics, tooling, and hardening.

This sequence gets the runtime model in place first, then grows compiler enforcement on top of it.

---

## Runtime Model

## Chosen V1 representation

Represent fallible results as dedicated runtime objects.

Suggested object kinds:

- `object_error_set_t`
- `object_error_value_t`
- `object_result_t`

Conceptually:

- `object_error_set_t`: identifies a named set like `FileError`
- `object_error_value_t`: identifies a specific member like `FileError.NotFound`
- `object_result_t`: tagged wrapper `{ isError, payload }`

Why this is the right V1 choice:

- least invasive to the existing `value_t` model
- easy to print/debug
- easy to pass through native function boundaries
- avoids immediate need for specialized bytecode-level union representation

Long-term optimization can remove wrappers where the compiler proves it safe.

---

## Phase 0: RFC Lock-In

Deliverables:

1. Final syntax for error sets and error unions.
2. Decision on whether `return FileError.NotFound;` is allowed as implicit error return inside fallible functions.
3. Decision on top-level handling rules for unhandled error results.
4. Decision on whether v1 supports unioned error sets like `(FileError | ParseError)!T`.
5. Decision on whether payload-bearing errors are explicitly deferred.

Recommended decisions:

- allow explicit `return error FileError.NotFound;` in v1
- optionally accept shorthand later
- no payload-bearing errors in v1
- support named error sets first, composed sets second

Exit criteria:

- syntax and semantics frozen before implementation starts

---

## Phase 1: Runtime Error Objects and Result Wrappers

Files likely impacted:

- [src/language-models/object.h](../src/language-models/object.h)
- [src/language-models/object.c](../src/language-models/object.c)
- [src/language-models/value.h](../src/language-models/value.h)
- [src/language-models/value.c](../src/language-models/value.c)
- [src/backend/garbage_collector.c](../src/backend/garbage_collector.c)

Tasks:

1. Add new object types:
   - error set object
   - error value object
   - result wrapper object
2. Extend object printing so errors/results are readable.
3. Extend equality semantics for error values and result wrappers.
4. Ensure garbage collector marks nested values inside result wrappers.
5. Add object constructors and helper APIs:
   - `object_new_error_set(...)`
   - `object_new_error_value(...)`
   - `object_new_result_success(...)`
   - `object_new_result_error(...)`

Exit criteria:

- runtime can create, print, compare, and collect error/result objects
- unit tests for value/object behavior pass

---

## Phase 2: Lexer + Parser Surface

Files:

- [src/frontend/lexical_analysis/lexer.h](../src/frontend/lexical_analysis/lexer.h)
- [src/frontend/lexical_analysis/lexer.c](../src/frontend/lexical_analysis/lexer.c)
- [src/frontend/parsing/statement_parser.c](../src/frontend/parsing/statement_parser.c)
- [src/frontend/parsing/expression_parser.c](../src/frontend/parsing/expression_parser.c)

New keywords/tokens:

- `error`
- `try`
- `catch`
- `must`
- `iferror`
- `else`
- `!` already exists as token; the parser must now also interpret it in type positions
- `->` return annotation syntax if not already introduced for functions

Tasks:

1. Add keyword recognition for new constructs.
2. Parse `error Name { A, B, C }` declarations.
3. Parse function return annotations for fallible returns.
4. Parse prefix `try expr`.
5. Parse infix/postfix `expr catch fallback`.
6. Parse prefix `must expr`.
7. Parse `iferror expr |err| { ... } else |value| { ... }`.
8. Parse `return error SomeSet.SomeVariant;`.

Notes:

- Parser may initially accept syntax before full semantic enforcement exists.

Exit criteria:

- syntax-level parser tests for all new forms pass
- malformed forms produce good parse errors

---

## Phase 3: Bytecode Lowering and VM Support

Files:

- [src/byte-code/chunk.h](../src/byte-code/chunk.h)
- [src/byte-code/chunk_disassembler.c](../src/byte-code/chunk_disassembler.c)
- [src/frontend/parsing/statement_parser.c](../src/frontend/parsing/statement_parser.c)
- [src/frontend/parsing/expression_parser.c](../src/frontend/parsing/expression_parser.c)
- [src/backend/virtual_machine.c](../src/backend/virtual_machine.c)

### Recommended V1 bytecode approach

Add explicit opcodes for result inspection and propagation rather than encoding everything as normal method calls.

Suggested new opcodes:

- `OP_ERROR_VALUE` — construct an error value
- `OP_RESULT_WRAP_OK` — wrap success value
- `OP_RESULT_WRAP_ERR` — wrap error value
- `OP_RESULT_IS_ERROR` — test whether a result is an error
- `OP_RESULT_UNWRAP` — extract success payload
- `OP_RESULT_UNWRAP_ERROR` — extract error payload
- `OP_TRY` — branch/return on error
- `OP_MUST` — fatal unwrap with runtime reporting

Alternative:

- lower everything into method calls on `Result` objects

Recommendation:

- use dedicated opcodes for `try` and `must`
- `catch` and `iferror` may lower to branches using `OP_RESULT_IS_ERROR`

Tasks:

1. Extend chunk opcode list.
2. Extend disassembler support.
3. Lower `try expr`:
   - evaluate expr
   - if error, return it from current function
   - else unwrap success value
4. Lower `catch`:
   - evaluate expr
   - branch on result kind
   - success -> unwrap value
   - error -> evaluate fallback/handler
5. Lower `must`:
   - evaluate expr
   - if error, print and terminate like runtime error
   - else unwrap
6. Lower `iferror`:
   - evaluate expr once
   - branch into error/success paths with bound local values
7. Support error value construction and result wrappers in the VM.

Exit criteria:

- hand-authored test programs using new syntax execute correctly
- VM can propagate, inspect, and unwrap results

---

## Phase 4: Semantic Fallibility Tracking and Forced Handling

This is the key phase for Zig-style discipline.

New modules suggested:

- `src/frontend/semantic/` for declaration collection and fallibility analysis

Tasks:

1. Add declaration prepass that records:
   - error set declarations
   - function names and fallible return annotations
2. Mark fallible expressions in semantic metadata.
3. Reject plain use of fallible expressions where handling is required.

Examples of errors to detect:

- assigning a fallible call directly to a variable without `try/catch/must/iferror`
- returning a fallible expression from an infallible function
- using `try` in an infallible function
- returning an error outside a fallible function
4. Validate error-set compatibility for propagation.
5. Validate `catch` handlers/fallback values are type-compatible enough for v1.

Because Cellox is still dynamic, v1 enforcement can be narrower:

- track fallibility structurally even if full type inference is absent
- focus on “must handle” enforcement first

Exit criteria:

- unhandled fallible expressions are compile-time errors
- `try`/`catch` misuse is reported clearly

---

## Phase 5: Native Function Boundary

Files:

- [src/backend/native_functions.h](../src/backend/native_functions.h)
- [src/backend/native_functions.c](../src/backend/native_functions.c)
- [src/backend/virtual_machine.c](../src/backend/virtual_machine.c)

Current issue:

- native functions return `value_t` only
- many failures are currently signaled through `null` or runtime error

Recommended V1 change:

- native functions continue to return `value_t`
- but may now return `OBJECT_VAL(resultWrapper)` to represent fallible results
- provide helper constructors/macros for success/error result creation

Tasks:

1. Add helper APIs/macros for native success/error returns.
2. Migrate clearly fallible native functions first:
   - file read/write/append
   - parsing-oriented utilities if present
   - system-facing functions where recoverable failure is meaningful
3. Leave true VM invariants as runtime errors.
4. Document the rule for native authors:
   - recoverable failure -> error result
   - invalid VM/program invariant -> runtime error

Exit criteria:

- at least one meaningful stdlib/native path uses error unions end-to-end
- no regression in existing native call behavior where migration has not happened yet

---

## Phase 6: Stdlib Migration

Files:

- [stdlib/io.clx](../stdlib/io.clx)
- future parsing/string/file modules

Tasks:

1. Convert I/O wrappers to explicit fallible APIs.
2. Add stdlib patterns showing recommended usage:
   - `try`
   - `catch`
   - `must`
   - `iferror`
3. Decide per API whether `null` or explicit error should be used.
4. Prefer error unions for:
   - file operations
   - parsing
   - environment/system interfaces with recoverable failure

Example migrations:

- `file_read(path) -> FileError!string`
- `file_write(path, text) -> FileError!bool`
- `read_number(msg) -> ParseError!number`

Exit criteria:

- stdlib has at least one well-designed end-to-end error-aware API family
- docs/examples demonstrate idiomatic usage

---

## Phase 7: Diagnostics and Tooling

Tasks:

1. Improve compiler diagnostics for unhandled fallible expressions.
2. Improve runtime diagnostics for `must` failures.
3. Print useful error names and origins.
4. Add disassembler support for new error-related bytecodes.
5. Document style guidance for stdlib authors.

Recommended diagnostics examples:

- `Unhandled fallible expression; use try, catch, must, or iferror.`
- `Can't use try in a function that does not return an error union.`
- `Returned error value is incompatible with declared error set.`

Exit criteria:

- diagnostics are actionable and consistent

---

## Test Plan

## Parser tests

1. Parse error set declarations.
2. Parse fallible function return syntax.
3. Parse `try`, `catch`, `must`, and `iferror`.
4. Reject malformed error syntax.

## Semantic tests

1. Reject unhandled fallible calls.
2. Reject `try` in infallible functions.
3. Reject error return from infallible functions.
4. Reject incompatible propagated error sets.

## VM/unit tests

1. Result wrapper creation and unwrapping.
2. Error value equality and printing.
3. `OP_TRY` propagation behavior.
4. `OP_MUST` fatal behavior.

## Integration tests

1. Compile and run a fallible file-read pipeline.
2. Native-function error result round trip.
3. Bytecode serialization compatibility if error opcodes are serialized.

## E2E tests

1. `try` propagation example.
2. `catch` fallback example.
3. `catch` handler example.
4. `iferror` branching example.
5. `must` fatal example.
6. stdlib file/parsing examples.

---

## Risks and Mitigations

1. Risk: Forced handling is difficult in a single-pass compiler.
Mitigation: add declaration/semantic prepass before full enforcement.

2. Risk: Result wrappers add runtime overhead.
Mitigation: accept wrapper model in v1; optimize later.

3. Risk: Native function migration becomes inconsistent.
Mitigation: define strict authoring rules and migrate module families together.

4. Risk: Confusion between `null` and error values.
Mitigation: document sharp separation and add tests enforcing API choices.

5. Risk: Too much syntax introduced at once.
Mitigation: phase parser support and keep v1 focused on `try/catch/must/iferror` only.

---

## Proposed Timeline (Rough)

1. Week 1: RFC lock-in and runtime representation
2. Week 2: lexer/parser surface
3. Week 3: bytecode lowering and VM support
4. Week 4: semantic fallibility tracking
5. Week 5: native + stdlib migration
6. Week 6: diagnostics, tests, docs, hardening

---

## Definition of Done

1. Error sets and error unions are supported.
2. `try`, `catch`, `must`, and `iferror` work end-to-end.
3. Unhandled fallible expressions are rejected at compile time.
4. Native functions can return explicit recoverable errors.
5. Stdlib exposes at least one idiomatic error-aware module family.
6. Fatal VM/runtime errors remain distinct from recoverable language errors.
7. Full test coverage exists across parser, semantic, VM, integration, and e2e layers.
