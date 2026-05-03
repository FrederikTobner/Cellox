# Cellox Static Library Refactoring Plan

## Objective

Refactor the current single large build into multiple static libraries with clear ownership and one-way dependencies.

Primary outcomes:
- Better component isolation and reasoning.
- Faster incremental builds.
- Lower risk of accidental cross-coupling.
- Stronger testability at library boundaries.

## Non-Goals

- No language feature changes.
- No VM behavior changes.
- No immediate public API redesign for external users.

## Current Architectural Risks

Observed dependency hotspots that create or encourage cycles:
- Runtime to frontend dependency: backend virtual machine calls compiler compile functionality.
- GC to frontend dependency: garbage collector calls compiler root marking directly.
- Runtime model to backend dependency: language model object definitions depend on native function declarations from backend.
- Bytecode to backend dependency: chunk constant insertion touches VM stack via backend APIs.

Build-system risks:
- Most binaries compile nearly all source files directly.
- Repeated source aggregation increases link and compile cost.
- Dependency contracts are implicit and easy to violate.

## Refactoring Principles

- Introduce strict layering and enforce one-way dependencies.
- Isolate cross-layer interactions behind narrow interface libraries.
- Keep headers local to owning library whenever possible.
- Treat compile and run as separate services connected by a stable contract.
- Migrate incrementally with passing tests at each phase.

## Target Library Layout

Proposed static libraries:

1. clx_base
- Purpose: foundational utilities and shared config.
- Files: common, string utils, lightweight shared helpers.
- Depends on: none.

2. clx_bytecode
- Purpose: chunk representation, serialization, disassembly primitives.
- Files: byte-code folder.
- Depends on: clx_base, clx_model.

3. clx_model
- Purpose: value and object model, hash tables, dynamic arrays.
- Files: language-models folder.
- Depends on: clx_base.

4. clx_memory
- Purpose: allocation and object lifetime primitives.
- Files: backend memory mutator and memory helpers split from GC.
- Depends on: clx_base, clx_model, clx_runtime_state.

5. clx_gc
- Purpose: mark-and-sweep collector.
- Files: backend garbage collector after state interfaces are extracted.
- Depends on: clx_base, clx_model, clx_memory, clx_runtime_state, clx_frontend_gc_api.

6. clx_frontend_lex
- Purpose: lexical analysis.
- Files: frontend lexical_analysis.
- Depends on: clx_base.

7. clx_frontend_parse
- Purpose: parser and expression/statement compilation logic.
- Files: frontend parsing and compilation context types.
- Depends on: clx_base, clx_model, clx_bytecode, clx_frontend_lex.

8. clx_frontend_compile
- Purpose: compiler orchestration entrypoints.
- Files: frontend compilation compiler and emit helpers.
- Depends on: clx_base, clx_model, clx_bytecode, clx_frontend_parse, clx_middleend.

9. clx_middleend
- Purpose: bytecode optimization passes.
- Files: middle-end folder.
- Depends on: clx_bytecode.

10. clx_runtime_state
- Purpose: VM state structures and access interfaces shared by VM, GC, and memory.
- Files: extracted runtime state headers and implementation.
- Depends on: clx_base, clx_model.

11. clx_runtime_exec
- Purpose: VM execution engine and native function dispatch.
- Files: backend virtual_machine and native_functions.
- Depends on: clx_base, clx_model, clx_bytecode, clx_memory, clx_gc, clx_runtime_state, clx_frontend_compile_api.

12. clx_app
- Purpose: interpreter startup and application glue.
- Files: initializer, module loader, command line parser.
- Depends on: clx_runtime_exec, clx_frontend_compile, clx_bytecode, clx_base.

## Planned Dependency Direction

Strict top-level direction:
- base -> model -> bytecode -> middleend
- base -> frontend_lex -> frontend_parse -> frontend_compile
- model + base -> runtime_state
- runtime_state + model + base -> memory -> gc
- runtime_exec depends on frontend_compile only via compile API, not internals
- app depends on runtime_exec and frontend_compile entry APIs

No reverse dependencies allowed.

## Required Interface Extractions to Break Cycles

### A. Compile Service Interface
Problem:
- Runtime execution currently depends on frontend compiler internals.

Action:
- Add frontend compile API header with opaque entrypoint for compile.
- Runtime links against this interface target only.
- Frontend compile implementation remains in frontend libraries.

Expected effect:
- Runtime no longer includes internal frontend headers.

### B. GC Root Provider Interface
Problem:
- Garbage collector directly calls compiler root marker.

Action:
- Add frontend GC API interface with function pointer registration.
- Compiler registers root marker at initialization.
- GC only calls registered callback, never frontend symbols directly.

Expected effect:
- GC has no direct compile-time dependency on frontend implementation.

### C. Native Function Type Relocation
Problem:
- Object model headers depend on backend native declarations.

Action:
- Move native function type alias to model-owned or runtime API neutral header.
- Keep backend native registry details inside runtime execution library.

Expected effect:
- Model library becomes backend-independent.

### D. Runtime State Split
Problem:
- GC and memory modules pull full virtual machine header.

Action:
- Extract runtime state struct and accessor API into runtime_state library.
- VM, memory, and GC all depend on runtime_state instead of each other.

Expected effect:
- Removes direct VM<->GC coupling at header level.

### E. Bytecode Constant Insertion Boundary
Problem:
- Bytecode module currently touches VM stack for GC safety during constant insertion.

Action:
- Introduce bytecode constant insertion callback or guard API exposed by runtime_state or memory interface.
- Bytecode code must not call VM functions directly.

Expected effect:
- Bytecode no longer depends on backend execution headers.

## CMake Refactor Strategy

### Phase 0: Foundation
- Keep existing executable targets unchanged.
- Create all library targets with empty or minimal source sets.
- Add target include directories and compile definitions per library.
- Introduce an option to build both legacy and refactored wiring during transition.

### Phase 1: Move Pure Leaf Libraries
- Build clx_base, clx_model, clx_bytecode, clx_middleend first.
- Link legacy executable against these libraries while still compiling old aggregate for parity checks.

### Phase 2: Frontend Libraries
- Split frontend into lex, parse, compile libraries.
- Add compile API interface target consumed by runtime and app.

### Phase 3: Runtime State, Memory, GC
- Extract runtime_state and move memory and GC behind it.
- Add GC root provider callback seam.

### Phase 4: Runtime Exec and App Wiring
- Build runtime_exec from VM and native functions.
- Rewire initializer and CLI to app library and final executable.

### Phase 5: Tool and Test Targets
- Link benchmark, disassembler, and tests against library graph instead of source glob lists.
- Remove duplicated source compilation from non-main executables.

### Phase 6: Cleanup
- Remove global source glob dependency for executable targets.
- Delete transitional compatibility shims.
- Enforce dependency direction checks.

## Suggested Target Linking Order

For main executable:
- clx_app
- clx_runtime_exec
- clx_frontend_compile
- clx_middleend
- clx_bytecode
- clx_gc
- clx_memory
- clx_runtime_state
- clx_model
- clx_base

For disassembler:
- clx_bytecode
- clx_model
- clx_base

For benchmark:
- clx_app
- clx_runtime_exec
- clx_frontend_compile
- clx_middleend
- clx_bytecode
- clx_gc
- clx_memory
- clx_runtime_state
- clx_model
- clx_base

## Validation and Safety Gates

After every phase:
- Build all targets in build-review.
- Run ctest labels e2e, integration, unit.
- Compare generated benchmark output for a fixed seed set.
- Run include-dependency check script to detect forbidden edges.

Additional checks:
- Ensure each library has explicit public headers and private headers.
- Ban direct includes across forbidden layers using CI grep guard.
- Ensure no executable directly compiles files from src once migration is complete.

## Incremental Milestones

Milestone 1:
- clx_base, clx_model, clx_bytecode, clx_middleend compile and link.

Milestone 2:
- frontend split complete and compile API consumed by runtime.

Milestone 3:
- runtime_state extracted; GC callback interface in place.

Milestone 4:
- runtime_exec plus app library complete; all tools and tests use libraries.

Milestone 5:
- legacy source aggregation removed; dependency policy enforced in CI.

## Risk Register

Risk: Hidden symbol coupling during split.
- Mitigation: Introduce interface headers first, then move implementations.

Risk: Static library link order issues.
- Mitigation: Use target_link_libraries with full dependency chain and keep dependency direction strict.

Risk: Performance regressions in GC-sensitive code paths.
- Mitigation: Keep benchmark baseline snapshots and compare each milestone.

Risk: Header include path breakage from ongoing include normalization.
- Mitigation: Keep includes rooted at src and avoid relative paths.

## Estimated Execution Plan

- Week 1: library skeletons and leaf libraries.
- Week 2: frontend split and compile API seam.
- Week 3: runtime_state and GC seam extraction.
- Week 4: runtime/app wiring, tool migration, and cleanup.

## Definition of Done

- All executables and tests link only against static libraries, not raw source globs.
- No cycle between frontend, runtime execution, GC, memory, model, and bytecode layers.
- e2e, integration, and unit test labels pass in CI.
- Benchmark and disassembler targets remain functional.
- Architecture and ownership documented in source and kept in sync with CMake.
