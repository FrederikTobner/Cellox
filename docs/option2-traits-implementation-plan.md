# Option 2 Implementation Plan: Traits/Typeclasses for Generic Views

## Objective

Implement Option 2 (traits/typeclasses with constraints) so `view` can work over arrays, stacks, queues, linked lists, and user-defined collections without duck typing.

Primary target:

- Generic algorithms constrained by traits, e.g. ViewSource and companion capabilities.
- Explicit implementations per type, e.g. impl ViewSource for Stack.
- View pipelines (map, filter, reduce, take, drop) operating on any trait-conforming source.
- Executable laws in traits for contract verification.
- Multi-method traits for binary and N-ary algorithm dispatch.
- Execution policy and algorithm properties to control behavior (parallelism, memory strategy, validation mode).

---

## Current Architecture Constraints (Important)

1. Compiler is currently single-pass and emits bytecode directly during parsing.
2. There is no separate semantic/type-check pass today.
3. Module export parser already supports `export class`, `export fun`, `export var` in [src/module_loader.c](../src/module_loader.c#L234).
4. Current parser/lexer do not include `trait`, `impl`, `for`, `where` tokens.
5. VM method dispatch is class/method hash-table based (OP_CLASS, OP_METHOD, OP_INVOKE) in [src/backend/virtual_machine.c](../src/backend/virtual_machine.c#L974).
6. There is no existing mechanism for law execution hooks or policy-aware algorithm lowering.

Implication:

- Full trait-bound compile-time checking requires introducing a semantic pass (or at least declaration/constraint prepass).

---

## Recommended Execution Strategy

Build Option 2 in 6 major stages:

1. Trait syntax + metadata collection.
2. Runtime trait dictionary dispatch (working feature).
3. Capability-oriented trait taxonomy + stdlib adaptation.
4. Compile-time constraint checking pass.
5. Executable laws and law execution modes.
6. Multi-method traits + execution policy/property framework.

This de-risks delivery while preserving the final Option 2 design.

---

## Language Design (Planned)

### New Declarations

```cellox
trait ViewSource<T> {
    size(source);
    at(source, index);

   // To what does this law apply? the at function, because of the signature?
   // Better to make this explicit using a special keyword like validates {index_in_range} for example that comes after the function declaration
   law index_in_range(source, i) {
      if (i >= 0 and i < size(source)) {
         at(source, i) != null;
      }
   }
}

trait SetAlgebra<A, B, Out> {
   union(a, b, policy);
   intersect(a, b, policy);
}

impl<T> ViewSource<T> for Stack<T> {
    size(s) { return s.size(); }
    at(s, i) { return s.at(i); }
}

class ExecPolicy {
   init(mode, allocation, law_mode) {
      this.mode = mode;           // seq, parallel, vector
      this.allocation = allocation; // default, arena, pooled
      this.law_mode = law_mode;   // off, sample, exhaustive
   }
}

fun view_of<T, S: ViewSource<T>>(source, policy) {
   // generic view pipeline entry point honoring policy
}
```

### Minimal grammar additions

- trait declaration
- impl declaration
- law declaration blocks can be inside traits, if they only should be usable for the current trait, but usually they are done on a global level
- multi-method trait signatures with multiple type parameters
- generic parameter list (<T, U>) on trait/function/class declarations
- constraint clause (S: ViewSource<T> and optional where)
- optional policy/property literal syntax for algorithms

---

## Runtime Model Choice

Use trait dictionaries first, then optimize later.

Dictionary model:

- Each impl becomes a dictionary object with function slots.
- Generic constrained call receives one or more hidden dictionary arguments.
- Multi-method call-sites resolve dictionary by concrete type tuple plus trait id.
- Execution policy object is passed explicitly and may influence algorithm branch selection.

Why this first:

- Avoids immediate monomorphization complexity.
- Fits current VM dynamic dispatch style.
- Easier incremental rollout.

Future optimization:

- Optional specialized codegen for hot generic instantiations.
- Optional specialization by policy (seq vs parallel) when optimizer can prove safety.

---

## Implementation Phases

## Phase 0: RFC Lock-In

Deliverables:

1. Final syntax decision for trait/impl/generic constraints.
2. Decision on nominal vs structural trait matching.
3. Dictionary layout spec and trait identity strategy.
4. Law semantics and execution modes (off/sample/exhaustive).
5. Multi-method coherence and ambiguity rules.
6. Policy object model and default behavior.

Proposed decision:

- Nominal traits (explicit `impl` required).
- No structural auto-conformance.
- Capability-oriented trait taxonomy for stdlib algorithms.
- Law execution default is off in production, sample in debug/tests.

Exit criteria:

- Short RFC accepted and frozen before code changes.

## Phase 1: Lexer + Parser Surface

Files:

- [src/frontend/lexical_analysis/lexer.h](../src/frontend/lexical_analysis/lexer.h)
- [src/frontend/lexical_analysis/lexer.c](../src/frontend/lexical_analysis/lexer.c)
- [src/frontend/parsing/statement_parser.c](../src/frontend/parsing/statement_parser.c)
- [src/frontend/parsing/expression_parser.c](../src/frontend/parsing/expression_parser.c)

Tasks:

1. Add tokens: TOKEN_TRAIT, TOKEN_IMPL, TOKEN_FOR, TOKEN_WHERE, TOKEN_LAW.
2. Extend keyword recognition in lexer.
3. Parse trait declarations and store signatures in compiler metadata.
4. Parse impl blocks and record target type + trait mapping.
5. Parse generic parameter lists and constraint clauses in function declarations.
6. Parse law blocks in trait declarations.
7. Parse policy/property arguments in algorithm signatures.

Notes:

- At this stage, parsing can accept syntax before full semantic enforcement.

Exit criteria:

- New syntax parses without crashes.
- Parser tests for valid/invalid syntax pass.

## Phase 2: Trait Metadata + Dictionary Emission

Files:

- [src/frontend/compilation/core_types.h](../src/frontend/compilation/core_types.h)
- [src/frontend/compilation/compilation_context.h](../src/frontend/compilation/compilation_context.h)
- [src/frontend/compilation/compilation_context.c](../src/frontend/compilation/compilation_context.c)
- [src/frontend/compilation/compiler.c](../src/frontend/compilation/compiler.c)
- [src/backend/virtual_machine.c](../src/backend/virtual_machine.c)

Tasks:

1. Add trait symbol tables to compilation context.
2. Add impl registry keyed by `(traitId, concreteTypeId)`.
3. Add multi-method impl registry keyed by `(traitId, typeIdTuple)`.
4. Emit dictionary objects/functions for each impl.
4. Add runtime helper for dictionary resolution by class/type.
5. Lower constrained generic calls to include hidden dictionary args.
6. Lower multi-method calls using tuple-based dictionary lookup.
7. Carry policy objects through lowered algorithm calls.

Design detail:

- Keep bytecode opcodes unchanged initially where possible.
- Reuse object/function invocation path via generated helper functions.

Exit criteria:

- End-to-end runtime behavior works for trait-constrained generic functions.
- Example: view_of(stack(), policy) runs through trait dictionary dispatch.

## Phase 3: Capability-Oriented Trait Taxonomy + Stdlib Alignment

Files:

- [stdlib/view.clx](../stdlib/view.clx)
- [stdlib/collections.clx](../stdlib/collections.clx)

Tasks:

1. Define base capabilities:
   - SizedSource
   - RandomAccessSource
   - SequenceSource
   - MutableSink (optional)
2. Define composed capability aliases for algorithm families.
3. Implement capability traits for Array wrapper, Stack, Queue, LinkedList.
4. Update view entrypoints to capability-constrained generic forms.

Exit criteria:

- Views and algorithms are written against capabilities rather than concrete structures.

## Phase 4: Semantic Constraint Checking Pass

This is the critical architecture step.

Files:

- Add new semantic modules under `src/frontend/semantic/`.
- Integrate from [src/frontend/compilation/compiler.c](../src/frontend/compilation/compiler.c).

Tasks:

1. Add declaration prepass:
   - collect traits
   - collect impl headers
   - collect generic signatures
2. Validate impl completeness:
   - all required trait members are implemented
   - signatures/arities match
3. Validate constrained generic calls:
   - argument types satisfy required trait bounds
4. Add duplicate/conflicting impl checks.
5. Add coherent error reporting messages.
6. Validate capability composition expansions.
7. Validate policy/property compatibility with algorithm requirements.

Why needed:

- Single-pass parser/compiler cannot reliably enforce trait constraints without pre-collected symbols.

Exit criteria:

- Compile-time errors for missing/bad impls.
- Compile-time errors for unsatisfied constraints at call sites.

## Phase 5: Executable Laws

Files:

- Add law execution support modules under src/frontend/semantic and src/backend.

Tasks:

1. Represent law AST and attach to trait metadata.
2. Build law runner generation for debug/test modes.
3. Add configurable law execution modes:
   - off
   - sample
   - exhaustive
4. Integrate law failure diagnostics with source location and values.
5. Add CI mode that executes laws for selected traits.

Exit criteria:

- Laws are executable and reported with actionable diagnostics.

## Phase 6: Multi-Method Traits + Execution Policy/Properties

Files:

- frontend semantic modules
- compiler lowering modules
- stdlib algorithm modules

Tasks:

1. Implement multi-method trait resolution for type tuples.
2. Add ambiguity checks and deterministic tie-breaking rules.
3. Define ExecPolicy and algorithm property schema for stdlib algorithms.
4. Implement policy-aware behavior knobs:
   - execution mode
   - allocation strategy
   - validation strictness
   - stability/order guarantees when relevant
5. Add optimization hooks for policy-driven specialization.

Exit criteria:

- Multi-method algorithms work with clear resolution semantics.
- Policies materially influence algorithm behavior in tested scenarios.

## Phase 7: Tooling, Diagnostics, and Hardening

Tasks:

1. Improve diagnostics for trait mismatch.
2. Add ambiguity/coherence checks for overlapping impls.
3. Add debug output for dictionary resolution (optional debug flag).
4. Document language feature in README/docs.
5. Add docs for laws, policy configuration, and capability traits.

---

## Test Plan

## Parser/Frontend tests

1. Parse valid trait declarations.
2. Reject malformed trait signatures.
3. Parse valid impl blocks.
4. Reject impl for unknown trait/type.
5. Parse generic function constraints.
6. Parse law declarations.
7. Parse multi-method trait signatures.
8. Parse policy/property argument constructs.

## Semantic tests

1. Missing trait method in impl fails.
2. Wrong impl arity/signature fails.
3. Generic constrained call with non-conforming type fails.
4. Duplicate impl conflict fails.
5. Law signature mismatch fails.
6. Ambiguous multi-method impl resolution fails.
7. Unsupported policy-property combination fails.

## E2E tests

1. view_of({1,2,3}, default_policy) pipeline works.
2. view_of(stack(), default_policy) pipeline works.
3. view_of(queue(), default_policy) pipeline works.
4. view_of(linked_list(), default_policy) pipeline works.
5. User-defined collection plus custom impl works.
6. Law failures surface meaningful diagnostics in test mode.
7. Multi-method algorithm dispatch chooses correct impl.
8. Policy variants alter behavior as configured.

## Integration tests

1. Module import/export with trait and impl declarations.
2. Trait dictionary survives compile + bytecode serialization + execute cycle.
3. Law execution mode toggles correctly by build/test settings.
4. Multi-method dictionary lookup survives serialization cycle.

## Performance tests

1. Compare baseline array-view vs trait-view overhead.
2. Benchmark map/filter/reduce over stack and array.
3. Benchmark policy modes (seq vs parallel or simulated parallel path).
4. Measure law execution overhead by mode.

---

## Risks and Mitigations

1. Risk: Complexity explosion from adding full static typing.
Mitigation: Keep dynamic runtime model and add constrained checks incrementally.

2. Risk: Single-pass compiler limitations.
Mitigation: Introduce declaration/semantic prepass before bytecode emission.

3. Risk: Ambiguous impl resolution.
Mitigation: Enforce nominal, non-overlapping impl rules initially.

4. Risk: Runtime overhead from dictionary dispatch.
Mitigation: Add optional specialization later for hot paths.

5. Risk: Law execution becomes expensive or flaky.
Mitigation: deterministic sampling strategy, explicit modes, and CI profiles.

6. Risk: Multi-method coherence complexity causes confusing errors.
Mitigation: strict non-overlap rules in first release, better diagnostics, then gradual relaxation.

7. Risk: Policy matrix explosion.
Mitigation: bounded policy schema with defaults and validation.

---

## Proposed Timeline (Rough)

1. Week 1-2: Phase 0 + Phase 1
2. Week 3-4: Phase 2 runtime dictionary support
3. Week 5: Phase 3 capability taxonomy and stdlib alignment
4. Week 6-7: Phase 4 semantic pass
5. Week 8: Phase 5 executable laws
6. Week 9-10: Phase 6 multi-method traits and execution policies
7. Week 11: Phase 7 hardening, docs, performance

---

## Definition of Done

1. Trait/typeclass syntax is stable and documented.
2. Constrained generic view API works for arrays and stack-like collections.
3. Compile-time diagnostics for unsatisfied constraints are implemented.
4. Executable laws are available with configurable execution modes.
5. Multi-method traits are supported with deterministic resolution rules.
6. Execution policy and algorithm properties are supported for core view algorithms.
7. Full test suite passes with new trait/generic/law/policy coverage.
8. Legacy array-only view path is either removed or formally deprecated.
